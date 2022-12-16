/**
 * @file main.c
 *
 * Run unit tests on linux.
 */

#include "sht4x.h"

#include "unity.h"

void app_main(void)
{
    UNITY_BEGIN();
    test_sht4x();
    UNITY_END();
}
