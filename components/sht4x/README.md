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

## Configuration

    $ idf.py menuconfig

In the `sht4x` menu, set the GPIO pins for your I²C SDA and SCL lines.
In most cases, you won't have to change the I²C master port number or
address.

## Usage

    #include "sht4x.h"

    void app_main(void)
    {
        float temperature, humidity;

        sht4x_init();

        while (1) {
            sht4x_measure(&temperature, &humidity);

            printf("%f %f\n", temperature, humidity);
            vTaskDelay(5000 / portTICK_PERIOD_MS);
        }
    }

In addition to `sht4x_measure()`, this component provides
`sht4x_measure_heat()` that activates the on-board heater of the
sensor before measuring the data.

````c
esp_err_t sht4x_heat_measure(sht4x_heat_t heat, float *temperature, float *humidity);
````

The parameter `heat` is one of the following options.

| `heat`              | Power (mW) | Duration (s) |
|---------------------|------------|--------------|
| SHT4X_HEAT_NONE     |          0 |            0 |
| SHT4X_HEAT_200_1000 |        200 |          1.0 |
| SHT4X_HEAT_200_100  |        200 |          0.1 |
| SHT4X_HEAT_110_1000 |        110 |          1.0 |
| SHT4X_HEAT_110_100  |        110 |          0.1 |
| SHT4X_HEAT_20_1000  |         20 |          1.0 |
| SHT4X_HEAT_20_100   |         20 |          0.1 |

## Contributing

[Pull requests][pulls] and [issue/bug reports][issues] are very much
encouraged!

## License

[MIT](LICENSE)


[issues]: https://github.com/bitmandu/sht4x/issues
[pulls]: https://github.com/bitmandu/sht4x/pulls
[1]: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/index.html
[2]: https://developer.sensirion.com/sensirion-products/sht4x-humidity-and-temperature-sensors/
[3]: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/tools/idf-component-manager.html
