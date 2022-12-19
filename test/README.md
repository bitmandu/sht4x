# sht4x/test

Unit tests for the [sht4x component][sht4x] that run on a linux
target.

## Build

    $ idf.py --preview set-target linux
    $ idf.py build

## Output

    $ ./build/sht4x_test.elf
    I (66655555) port: Starting scheduler.
    Running tests matching '[sht4x]'...
    ...
    -----------------------
    12 Tests 0 Failures 0 Ignored

## Help / Contributing

[Bug reports][issues] and [pull requests][pulls] are very much
encouraged.


[issues]: https://github.com/bitmandu/sht4x/issues
[pulls]: https://github.com/bitmandu/sht4x/pulls
[sht4x]: https://github.com/bitmandu/sht4x/tree/main/components/sht4x
