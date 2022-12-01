/**
 * @file sht4x.h
 *
 * Sensirion SHT4x relative humidity and temperature sensor.
 */

#pragma once

/**< Heater activation options; see datasheet Table 7 */
typedef enum {
    SHT4X_HEAT_NONE, // no heater
    SHT4X_HEAT_200_1000, // activate with 200 mW for 1.0 s
    SHT4X_HEAT_200_100, // activate with 200 mW for 0.1 s
    SHT4X_HEAT_110_1000, // activate with 110 mW for 1.0 s
    SHT4X_HEAT_110_100, // activate with 110 mW for 0.1 s
    SHT4X_HEAT_20_1000, // activate with  20 mW for 1.0 s
    SHT4X_HEAT_20_100 // activate with  20 mW for 0.1 s
} sht4x_heat_t;

/**
 * Initialize SHT4x using menuconfig parameters.
 */
esp_err_t sht4x_init(void);

/**
 * Soft reset sensor.
 *
 * @return ESP_OK on succcess.
 */
esp_err_t sht4x_reset(void);

/**
 * Measure temperature and humidity.
 *
 * @param temp Temperature (°C)
 * @param humidity Relative humidity in [0.0, 100.0]
 *
 * @return ESP_OK on success.
 */
esp_err_t sht4x_measure(float *temp, float *humidity);

/**
 * Measure temperature and humidity after heater activation (if any).
 *
 * @param heat Heater activation option
 * @param temp Temperature (°C)
 * @param humidity Relative humidity in [0.0, 100.0]
 *
 * @return ESP_OK on success.
 */
esp_err_t sht4x_heat_measure(sht4x_heat_t heat, float *temp, float *humidity);
