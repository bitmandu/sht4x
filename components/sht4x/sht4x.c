/**
 * @file sht4x.c
 *
 * Sensirion SHT4x relative humidity and temperature sensor.
 */

#include <stdbool.h>

#include "esp_log.h"
#include "esp_check.h"

#include "driver/i2c.h"

#include "sht4x.h"

static const char *TAG = "sht4x";

#define SHT4X_CMD_SERIAL 0x89
#define SHT4X_CMD_RESET 0x94
static const uint8_t SHT4X_CMD_MEASURE[] = {
    [SHT4X_HEAT_NONE] = 0xfd,    [SHT4X_HEAT_200_1000] = 0x39,
    [SHT4X_HEAT_200_100] = 0x32, [SHT4X_HEAT_110_1000] = 0x2f,
    [SHT4X_HEAT_110_100] = 0x24, [SHT4X_HEAT_20_1000] = 0x1e,
    [SHT4X_HEAT_20_100] = 0x15};

#define G_POLYNOM 0x31

/** CRC checksum. */
static uint8_t crc8(uint8_t *data, size_t len)
{
    uint8_t crc = 0xff;

    for (size_t i = 0; i < len; ++i) {
        crc ^= data[i];

        for (int k = 0; k < 8; ++k) {
            crc = crc & 0x80 ? (crc << 1) ^ G_POLYNOM : crc << 1;
        }
    }

    return crc;
}

/** Validate data using CRC checksum. Returns 0 if both CRC checksums match. */
static int invalid_crc(uint8_t *data)
{
    return (crc8(data, 2) ^ data[2]) | (crc8(&data[3], 2) ^ data[5]);
}

/** Read delay corresponding to heating option. */
static TickType_t heat_delay(sht4x_heat_t heat)
{
    TickType_t delay;

    // datasheet pseudocode has a wait delay of 0.01 sec = 10 ms

    switch (heat) {
    case SHT4X_HEAT_NONE:
        delay = 10;
        break;

    case SHT4X_HEAT_200_1000:
    case SHT4X_HEAT_110_1000:
    case SHT4X_HEAT_20_1000:
        delay = 1010;
        break;

    case SHT4X_HEAT_200_100:
    case SHT4X_HEAT_110_100:
    case SHT4X_HEAT_20_100:
        delay = 110;
        break;

    default:
        ESP_LOGE(TAG, "unknown heating option: %d", heat);
        delay = 0;
        break;
    }

    return delay;
}

/** Send command and read response data. */
static esp_err_t sht4x_write_read(uint8_t cmd, uint8_t *data, size_t len, TickType_t delay)
{
    esp_err_t err;

    err = i2c_master_write_to_device(CONFIG_SHT4X_PORT, CONFIG_SHT4X_ADDR, &cmd,
                                     1, portMAX_DELAY);
    ESP_RETURN_ON_ERROR(err, TAG, "i2c_master_write_to_device");

    vTaskDelay(delay / portTICK_PERIOD_MS);

    return i2c_master_read_from_device(CONFIG_SHT4X_PORT, CONFIG_SHT4X_ADDR,
                                       data, len, portMAX_DELAY);
}

/** Read SHT4x device serial number. */
static esp_err_t sht4x_read_serial(uint32_t *serial)
{
    uint8_t data[6];
    esp_err_t err;

    err = sht4x_write_read(SHT4X_CMD_SERIAL, data, sizeof(data), 10);
    ESP_RETURN_ON_ERROR(err, TAG, "sht4x_write_read");

    if (invalid_crc(data)) {
        return ESP_ERR_INVALID_CRC;
    }

    *serial = ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) |
              ((uint32_t)data[3] << 8) | data[4];

    return ESP_OK;
}

esp_err_t sht4x_init(void)
{
    uint32_t serial;
    esp_err_t err;
    i2c_config_t config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = CONFIG_SHT4X_SDA,
        .scl_io_num = CONFIG_SHT4X_SCL,
        .master.clk_speed = 100000,
    };

    err = i2c_param_config(CONFIG_SHT4X_PORT, &config);
    ESP_RETURN_ON_ERROR(err, TAG, "i2c_param_config");

    err = i2c_driver_install(CONFIG_SHT4X_PORT, config.mode, 0, 0, 0);
    ESP_RETURN_ON_ERROR(err, TAG, "i2c_driver_install");

    while (sht4x_read_serial(&serial) != ESP_OK) {
        ESP_LOGI(TAG, "... waiting for SHT4x");
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }

    ESP_LOGI(TAG, "serial number 0x%08" PRIx32, serial);
    return ESP_OK;
}

esp_err_t sht4x_reset(void)
{
    uint8_t cmd[] = {SHT4X_CMD_RESET};

    return i2c_master_write_to_device(CONFIG_SHT4X_PORT, CONFIG_SHT4X_ADDR, cmd,
                                      sizeof(cmd), portMAX_DELAY);
}

esp_err_t sht4x_measure(float *temp, float *humidity)
{
    return sht4x_heat_measure(SHT4X_HEAT_NONE, temp, humidity);
}

esp_err_t sht4x_heat_measure(sht4x_heat_t heat, float *temp, float *humidity)
{
    uint32_t t, rh;
    uint8_t data[6];
    esp_err_t err;
    TickType_t delay;

    delay = heat_delay(heat);
    if (!delay) {
        return ESP_ERR_INVALID_ARG;
    }

    err = sht4x_write_read(SHT4X_CMD_MEASURE[heat], data, sizeof(data), delay);
    ESP_RETURN_ON_ERROR(err, TAG, "sht4x_write_read");

    if (invalid_crc(data)) {
        return ESP_ERR_INVALID_CRC;
    }

    t = ((uint32_t)(data[0] << 8)) | data[1];
    rh = ((uint32_t)(data[3] << 8)) | data[4];
    ESP_LOGI(TAG,
             "measurement: %02x %02x %02x %02x %02x %02x: "
             "t=%" PRIu32 ", rh=%" PRIu32 " ticks",
             data[0], data[1], data[2], data[3], data[4], data[5], t, rh);

    *temp = -45.0 + 175.0 * t / 65535.0;
    *humidity = -6.0 + 125.0 * rh / 65535.0;

    // crop humidity to [0, 100]; see datasheet § 4.5
    *humidity = (*humidity > 100.0) ? 100.0 : *humidity;
    *humidity = (*humidity < 0.0) ? 0.0 : *humidity;

    return ESP_OK;
}
