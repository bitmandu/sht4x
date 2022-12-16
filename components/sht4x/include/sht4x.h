/**
 * @file sht4x.h
 *
 * Sensirion SHT4x relative humidity and temperature sensor driver.
 */

#pragma once

#include "esp_err.h"
#include "driver/i2c.h"

/** Type for SHT4X object handle. */
typedef struct sht4x *sht4x_t;

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
 * Initialize SHT4x sensor.
 *
 * @param port I2C port number
 * @param address I2C device address
 * @param sht4x Sensor handle
 *
 * @return ESP_OK on success.
 */
esp_err_t sht4x_init(i2c_port_t port, uint8_t address, sht4x_t *sht4x);

/**
 * Soft reset sensor.
 *
 * @param sht4x Sensor handle
 *
 * @return ESP_OK on success.
 */
esp_err_t sht4x_reset(sht4x_t sht4x);

/**
 * Get SHT4x device serial number.
 *
 * The two 16-bit words for the serial number are returned as a 32-bit
 * integer, where the first (second) 16-bit word is in MSB (LSB)
 * position.
 *
 * @param sht4x Sensor handle
 * @param serial Serial number
 *
 * @return ESP_OK on success.
 */
esp_err_t sht4x_get_serial(sht4x_t sht4x, uint32_t *serial);

/**
 * Measure temperature and humidity.
 *
 * @param sht4x Sensor handle
 * @param temperature Temperature (°C)
 * @param humidity Relative humidity in [0.0, 100.0]
 *
 * @return ESP_OK on success.
 */
esp_err_t sht4x_measure(sht4x_t sht4x, float *temperature, float *humidity);

/**
 * Measure temperature and humidity after heater activation (if any).
 *
 * @param sht4x Sensor handle
 * @param heat Heater activation option
 * @param temperature Temperature (°C)
 * @param humidity Relative humidity in [0.0, 100.0]
 *
 * @return ESP_OK on success.
 */
esp_err_t sht4x_heat_measure(sht4x_t sht4x, sht4x_heat_t heat,
                             float *temperature, float *humidity);

/**
 * Measure raw temperature and humidity data.
 *
 * @param sht4x Sensor handle
 * @param temperature Temperature in [0, 0xffff)
 * @param humidity Relative humidity in [0, 0xffff)
 *
 * @return ESP_OK on success.
 */
esp_err_t sht4x_measure_raw(sht4x_t sht4x, uint32_t *temperature, uint32_t *humidity);

/**
 * Measure raw temperature and humidity data after heater activation (if any).
 *
 * @param sht4x Sensor handle
 * @param heat Heater activation option
 * @param temperature Temperature in [0, 0xffff)
 * @param humidity Relative humidity in [0, 0xffff)
 *
 * @return ESP_OK on success.
 */
esp_err_t sht4x_heat_measure_raw(sht4x_t sht4x, sht4x_heat_t heat,
                                 uint32_t *temperature, uint32_t *humidity);

/**
 * Deallocate memory.
 *
 * @param sht4x Sensor handle
 */
void sht4x_delete(sht4x_t sht4x);

/**
 * Run unit tests.
 */
void test_sht4x(void);
