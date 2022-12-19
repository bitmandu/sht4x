# sht4x

This is an [ESP-IDF][1] component for the [Sensirion SHT4x][2]
temperature and humidity sensor.

## Installation

To install this component using the [IDF Component Manager][3], add
the following to your `idf_component.yml`:

    dependencies:
      sht4x:
        git: https://github.com/bitmandu/sht4x.git
        path: components/sht4x

Then update the managed components.

    $ idf.py reconfigure

## Usage

An [example application][main] shows all the details, but here's the gist:

    #include "sht4x.h"
    #include "driver/i2c.h"

    #define SDA GPIO_NUM_6
    #define SDL GPIO_NUM_5
    #define PORT I2C_NUM_0

    void app_main(void)
    {
        sht4x_t sht4x;
        float temperature, humidity;

        const i2c_config_t config = {.mode = I2C_MODE_MASTER,
                                     .sda_io_num = SDA,
                                     .scl_io_num = SDL,
                                     .master.clk_speed = 1000000};
        ESP_ERROR_CHECK(i2c_param_config(PORT, &config));
        ESP_ERROR_CHECK(i2c_driver_install(PORT, config.mode, 0, 0, 0));

        ESP_ERROR_CHECK(sht4x_init(PORT, CONFIG_SHT4X_ADDRESS, &sht4x));

        while (1) {
            sht4x_measure(sht4x, &temperature, &humidity);

            printf("%f %f\n", temperature, humidity);
            vTaskDelay(5000 / portTICK_PERIOD_MS);
        }
    }

In addition to `sht4x_measure()`, this component provides
`sht4x_measure_heat()` that activates the on-board heater of the
sensor before doing a measurement.

````c
esp_err_t sht4x_heat_measure(sht4x_t sht4x, sht4x_heat_t heat, float *temperature, float *humidity);
````

The parameter `heat` has the following options.

| `heat`              | Power (mW) | Duration (s) |
|---------------------|------------|--------------|
| SHT4X_HEAT_NONE     | 0          | 0            |
| SHT4X_HEAT_200_1000 | 200        | 1.0          |
| SHT4X_HEAT_200_100  | 200        | 0.1          |
| SHT4X_HEAT_110_1000 | 110        | 1.0          |
| SHT4X_HEAT_110_100  | 110        | 0.1          |
| SHT4X_HEAT_20_1000  | 20         | 1.0          |
| SHT4X_HEAT_20_100   | 20         | 0.1          |

## Help / Contributing

[Bug reports][issues] and [pull requests][pulls] are very much
encouraged.

## License

[MIT](LICENSE)


[issues]: https://github.com/bitmandu/sht4x/issues
[pulls]: https://github.com/bitmandu/sht4x/pulls
[1]: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/index.html
[2]: https://developer.sensirion.com/sensirion-products/sht4x-humidity-and-temperature-sensors/
[3]: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/tools/idf-component-manager.html
[main]: https://github.com/bitmandu/sht4x/blob/main/main/main.c
