/**
 * @file main.c
 *
 * Test application for Sensirion SHT4x sensor.
 */

#include "sht4x.h"

#include "esp_log.h"
#include "driver/i2c.h"

#include "freertos/FreeRTOS.h"

#include <stdio.h>

#define SDA GPIO_NUM_6
#define SDL GPIO_NUM_5
#define PORT I2C_NUM_0

static const char *TAG = "main";

void app_main(void)
{
    ESP_LOGI(TAG, "version 0.1.0");

    const i2c_config_t config = {.mode = I2C_MODE_MASTER,
                                 .sda_io_num = SDA,
                                 .scl_io_num = SDL,
                                 .master.clk_speed = 1000000};
    ESP_ERROR_CHECK(i2c_param_config(PORT, &config));
    ESP_ERROR_CHECK(i2c_driver_install(PORT, config.mode, 0, 0, 0));

    sht4x_t sht4x;
    ESP_ERROR_CHECK(sht4x_init(PORT, CONFIG_SHT4X_ADDRESS, &sht4x));

    float temperature, humidity;
    uint32_t n = 0;

    while (1) {
        if (n == 3) {
            ESP_LOGI(TAG, "heating....");
            sht4x_heat_measure(sht4x, SHT4X_HEAT_20_100, &temperature, &humidity);
        } else {
            sht4x_measure(sht4x, &temperature, &humidity);
        }

        printf("** %" PRIu32 " %f %f\n", n, temperature, humidity);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        ++n;
    }
}
