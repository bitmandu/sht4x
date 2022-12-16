/**
 * @file sht4x.c
 *
 * Sensirion SHT4x relative humidity and temperature sensor driver.
 */

#include "sht4x.h"

#include "esp_log.h"
#include "esp_check.h"
#include "driver/i2c.h"

#include <stdbool.h>

static const char *TAG = "sht4x";

struct sht4x {
    uint32_t serial;
    i2c_port_t port;
    uint8_t address;
};

#define SHT4X_NUM_RETRY 3
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

/**
 * Validate CRC of 6-byte payload {data, data, CRC, data, data, CRC}.
 * Returns true if both CRC checksums match.
 */
static bool valid_crc(uint8_t *data)
{
    return (crc8(data, 2) == data[2]) && (crc8(&data[3], 2) == data[5]);
}

static float raw_to_temperature(uint32_t raw)
{
    return -45.0 + 175.0 * raw / 65535.0;
}

static float raw_to_relative_humidity(uint32_t raw)
{
    float rh;

    rh = -6.0 + 125.0 * raw / 65535.0;

    // crop humidity to [0, 100]; see datasheet § 4.5
    rh = (rh > 100.0f) ? 100.0f : rh;
    rh = (rh < 0.0f) ? 0.0f : rh;

    return rh;
}

/** Read delay (in ms) corresponding to heating option. */
static TickType_t heat_delay(sht4x_heat_t heat)
{
    TickType_t delay;

    // see datasheet, Table 4; only using "high repeatability"
    // measurments that take ~10 ms (+ heating time)

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
static esp_err_t sht4x_write_read(sht4x_t sht4x, uint8_t cmd, uint8_t *data,
                                  size_t len, TickType_t delay_ms)
{
    int num_retry = SHT4X_NUM_RETRY;

    do {
        ESP_RETURN_ON_ERROR(i2c_master_write_to_device(sht4x->port, sht4x->address,
                                                       &cmd, 1, portMAX_DELAY),
                            TAG, "i2c_master_write_to_device");

        vTaskDelay(delay_ms / portTICK_PERIOD_MS);

        ESP_RETURN_ON_ERROR(i2c_master_read_from_device(sht4x->port, sht4x->address,
                                                        data, len, portMAX_DELAY),
                            TAG, "i2c_master_read_from_device");

        if (valid_crc(data)) {
            return ESP_OK;
        }

        ESP_LOGE(TAG, "... retrying to read from sensor");
        vTaskDelay(10 / portTICK_PERIOD_MS);
    } while (--num_retry);

    return ESP_ERR_TIMEOUT;
}

/** Read serial number from sensor. */
static esp_err_t sht4x_read_serial(sht4x_t sht4x, uint32_t *serial)
{
    uint8_t data[6];

    ESP_RETURN_ON_ERROR(sht4x_write_read(sht4x, SHT4X_CMD_SERIAL, data,
                                         sizeof(data), 10),
                        TAG, "sht4x_write_read");

    *serial = ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) |
              ((uint32_t)data[3] << 8) | data[4];

    return ESP_OK;
}

esp_err_t sht4x_init(i2c_port_t port, uint8_t address, sht4x_t *handle)
{
    struct sht4x *sht4x;
    esp_err_t ret;

    sht4x = malloc(sizeof(*sht4x));
    if (!sht4x) {
        *handle = NULL;
        return ESP_ERR_NO_MEM;
    }

    sht4x->port = port;
    sht4x->address = address;

    vTaskDelay(1); // SHT4x needs 1 ms to power on (1 tick ~= 10 ms)
    ret = sht4x_read_serial(sht4x, &sht4x->serial);

    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "found device 0x%08" PRIx32, sht4x->serial);
    } else if (ret == ESP_ERR_TIMEOUT) {
        ESP_LOGE(TAG, "timeout reading serial number: address=0x%02x", sht4x->address);
    }

    *handle = sht4x;
    return ret;
}

esp_err_t sht4x_get_serial(sht4x_t sht4x, uint32_t *serial)
{
    if (!sht4x) {
        return ESP_ERR_INVALID_ARG;
    }

    *serial = sht4x->serial;
    return ESP_OK;
}

esp_err_t sht4x_reset(sht4x_t sht4x)
{
    uint8_t cmd[] = {SHT4X_CMD_RESET};
    esp_err_t ret;

    if (!sht4x) {
        return ESP_ERR_INVALID_ARG;
    }

    ret = i2c_master_write_to_device(sht4x->port, sht4x->address, cmd,
                                     sizeof(cmd), portMAX_DELAY);
    vTaskDelay(1); // SHT4x needs 1 ms to reset (1 tick ~= 10 ms)
    return ret;
}

void sht4x_delete(sht4x_t sht4x)
{
    free(sht4x);
}

esp_err_t sht4x_heat_measure_raw(sht4x_t sht4x, sht4x_heat_t heat,
                                 uint32_t *temp, uint32_t *humidity)
{
    uint8_t data[6];
    TickType_t delay_ms;

    delay_ms = heat_delay(heat);

    if (!sht4x || !delay_ms) {
        return ESP_ERR_INVALID_ARG;
    }

    ESP_RETURN_ON_ERROR(sht4x_write_read(sht4x, SHT4X_CMD_MEASURE[heat], data,
                                         sizeof(data), delay_ms),
                        TAG, "sht4x_write_read");

    *temp = ((uint32_t)(data[0] << 8)) | data[1];
    *humidity = ((uint32_t)(data[3] << 8)) | data[4];
    ESP_LOGD(TAG,
             "measurement: %02x %02x %02x %02x %02x %02x: "
             "t=%" PRIu32 ", rh=%" PRIu32,
             data[0], data[1], data[2], data[3], data[4], data[5], *temp, *humidity);

    return ESP_OK;
}

esp_err_t sht4x_heat_measure(sht4x_t sht4x, sht4x_heat_t heat, float *temp, float *humidity)
{
    uint32_t t, rh;

    if (!sht4x) {
        return ESP_ERR_INVALID_ARG;
    }

    ESP_RETURN_ON_ERROR(sht4x_heat_measure_raw(sht4x, heat, &t, &rh), TAG,
                        "sht4x_heat_measure_raw: heat=%d", heat);

    *temp = raw_to_temperature(t);
    *humidity = raw_to_relative_humidity(rh);
    return ESP_OK;
}

esp_err_t sht4x_measure_raw(sht4x_t sht4x, uint32_t *temp, uint32_t *humidity)
{
    return sht4x_heat_measure_raw(sht4x, SHT4X_HEAT_NONE, temp, humidity);
}

esp_err_t sht4x_measure(sht4x_t sht4x, float *temp, float *humidity)
{
    return sht4x_heat_measure(sht4x, SHT4X_HEAT_NONE, temp, humidity);
}
