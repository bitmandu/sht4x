# sht4x

This is an example [ESP-IDF][1] application for the [Sensirion
SHT4x][2] temperature and humidity sensor.

## Configuration

    $ idf.py menuconfig

In the `sht4x` menu, set the GPIO pins for your I²C SDA and SCL lines.

In most cases, you won't have to change the I²C master port number or
device address.

## Example output

    $ idf.py build flash monitor

    ESP-ROM:esp32s2-rc4-20191025
    Build:Oct 25 2019
    rst:0x1 (POWERON),boot:0x8 (SPI_FAST_FLASH_BOOT)
    ...
    I (278) main: version 0.0
    I (288) sht4x: serial number 0x108906f5
    I (298) sht4x: measurement: 63 37 ab 6a 6c fc: t=25399, rh=27244 ticks
    ** 0 22.823683 45.964600
    I (5308) sht4x: measurement: 63 24 bb 6a d7 a9: t=25380, rh=27351 ticks
    ** 1 22.772945 46.168690
    I (10318) sht4x: measurement: 63 22 1d 6b 1c f0: t=25378, rh=27420 ticks
    ** 2 22.767605 46.300297
    I (15318) main: heating....
    I (16328) sht4x: measurement: b3 b4 46 4a a4 de: t=46004, rh=19108 ticks
    ** 3 77.845810 30.446175
    I (21338) sht4x: measurement: 63 f4 bf 58 24 13: t=25588, rh=22564 ticks
    ** 4 23.328375 37.038071

On the third iteration, the heating capibility of the SHT4x sensor is
activated.

A Python script is provided to visualize the temperature and humidity in real-time.

    $ ./scripts/plot.py

![Real-time SHT4x sensor data](scripts/example-plot.png "Real-time SHT4x sensor data")

## Component installation

See the [component documunation](components/sht4x/README.md) to use
this code as a component in your own project.

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
