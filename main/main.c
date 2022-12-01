/**
 * @file main.c
 *
 * Test application for Sensirion SHT4x sensor.
 */

#include <stdio.h>
#include <sys/time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "sht4x.h"

static const char *TAG = "main";

void app_main(void)
{
    float temperature, humidity;
    int n = 0;

    ESP_LOGI(TAG, "version 0.0");

    ESP_ERROR_CHECK(sht4x_init());

    while (1) {
        if (n == 3) {
            ESP_LOGI(TAG, "heating....");
            sht4x_heat_measure(SHT4X_HEAT_20_100, &temperature, &humidity);
        } else {
            sht4x_measure(&temperature, &humidity);
        }

        printf("** %d %f %f\n", n, temperature, humidity);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        n++;
    }
}
