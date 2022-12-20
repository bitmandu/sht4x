/**
 * @file test_sht4x.c
 *
 * Test Sensirion SHT4x relative humidity and temperature sensor driver.
 */

#include "sht4x.h"
#include "driver/stub_i2c.h"
#include "driver/mock_i2c.h"

#include "unity.h"

#include <string.h>

static sht4x_t sht4x;

#define SHT4X_CMD_SERIAL 0x89
#define SHT4X_CMD_MEASURE 0xfd
#define SHT4X_CMD_RESET 0x94

#define SDA GPIO_NUM_6
#define SDL GPIO_NUM_5
#define PORT I2C_NUM_0

#define DELTA 1.0e-6

static const uint8_t *read_cb_data;

/** Callback function called after i2c_master_read_from_device() mock */
static esp_err_t read_cb(i2c_port_t port, uint8_t address, uint8_t *buffer,
                         size_t len, TickType_t ticks_to_wait, esp_err_t num_call)
{
    if (!buffer || !read_cb_data || !len) {
        return ESP_FAIL;
    }

    memcpy(buffer, read_cb_data, len);
    return ESP_OK;
}

static void setup()
{
    const i2c_config_t config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = SDA,
        .scl_io_num = SDL,
        .master.clk_speed = 100000,
    };

    TEST_ASSERT_EQUAL(ESP_OK, i2c_param_config(PORT, &config));
    TEST_ASSERT_EQUAL(ESP_OK, i2c_driver_install(PORT, config.mode, 0, 0, 0));

    const uint8_t cmd = SHT4X_CMD_SERIAL;
    const uint8_t serial_num[] = {0xde, 0xad, 0x98, 0xbe, 0xef, 0x92};

    read_cb_data = serial_num;

    mock_i2c_Init();

    i2c_master_write_to_device_ExpectAndReturn(PORT, CONFIG_SHT4X_ADDRESS, &cmd,
                                               1, portMAX_DELAY, ESP_OK);
    i2c_master_read_from_device_ExpectAndReturn(PORT, CONFIG_SHT4X_ADDRESS,
                                                NULL, 6, portMAX_DELAY, ESP_OK);
    i2c_master_read_from_device_IgnoreArg_read_buffer();
    i2c_master_read_from_device_AddCallback(read_cb);

    TEST_ASSERT_EQUAL(ESP_OK, sht4x_init(PORT, CONFIG_SHT4X_ADDRESS, &sht4x));
}

static void teardown()
{
    mock_i2c_Verify();
    mock_i2c_Destroy();

    stub_i2c_reset();
    sht4x_delete(sht4x);
    sht4x = NULL;
}

TEST_CASE("sht4x_init() should return handle", "[sht4x]")
{
    setup();
    TEST_ASSERT(sht4x);
    teardown();
}

TEST_CASE("sht4x_init() should find serial number", "[sht4x]")
{
    const uint32_t expected = 0xdeadbeef;
    uint32_t actual;

    setup();
    TEST_ASSERT_EQUAL(ESP_OK, sht4x_get_serial(sht4x, &actual));
    TEST_ASSERT_EQUAL_HEX32(expected, actual);
    teardown();
}

TEST_CASE("sht4x_reset() should send reset command", "[sht4x]")
{
    uint8_t cmd = SHT4X_CMD_RESET;

    setup();
    i2c_master_write_to_device_ExpectAndReturn(PORT, CONFIG_SHT4X_ADDRESS, &cmd,
                                               1, portMAX_DELAY, ESP_OK);
    TEST_ASSERT_EQUAL(ESP_OK, sht4x_reset(sht4x));
    teardown();
}

TEST_CASE("sht4x_heat_measure_raw() should return temperature and humidity", "[sht4x]")
{
    uint8_t cmd = SHT4X_CMD_MEASURE;
    const uint8_t data[] = {0x5f, 0x16, 0x1a, 0x5e, 0x35, 0x3b};
    uint32_t temp_expected = 0x5f16;
    uint32_t rh_expected = 0x5e35;
    uint32_t temp, rh;

    setup();

    read_cb_data = data;
    i2c_master_write_to_device_ExpectAndReturn(PORT, CONFIG_SHT4X_ADDRESS, &cmd,
                                               1, portMAX_DELAY, ESP_OK);
    i2c_master_read_from_device_ExpectAndReturn(PORT, CONFIG_SHT4X_ADDRESS,
                                                NULL, 6, portMAX_DELAY, ESP_OK);
    i2c_master_read_from_device_IgnoreArg_read_buffer();
    i2c_master_read_from_device_AddCallback(read_cb);

    TEST_ASSERT_EQUAL(ESP_OK,
                      sht4x_heat_measure_raw(sht4x, SHT4X_HEAT_NONE, &temp, &rh));
    TEST_ASSERT_EQUAL_HEX32(temp_expected, temp);
    TEST_ASSERT_EQUAL_HEX32(rh_expected, rh);
    teardown();
}

TEST_CASE("sht4x_heat_measure_raw() should timeout if unable to read data from sensor",
          "[sht4x]")
{
    uint8_t cmd = SHT4X_CMD_MEASURE;
    const uint8_t bad_crc[] = {0x5f, 0x16, 0xff, 0x5e, 0x35, 0xff};
    uint32_t temp, rh;

    setup();

    read_cb_data = bad_crc;
    i2c_master_read_from_device_AddCallback(read_cb);

    for (int i = 0; i <= CONFIG_SHT4X_NUM_RETRY; ++i) {
        i2c_master_write_to_device_ExpectAndReturn(PORT, CONFIG_SHT4X_ADDRESS,
                                                   &cmd, 1, portMAX_DELAY, ESP_OK);
        i2c_master_read_from_device_ExpectAndReturn(PORT, CONFIG_SHT4X_ADDRESS,
                                                    NULL, 6, portMAX_DELAY, ESP_OK);
        i2c_master_read_from_device_IgnoreArg_read_buffer();
    }

    TEST_ASSERT_EQUAL(ESP_ERR_TIMEOUT,
                      sht4x_heat_measure_raw(sht4x, SHT4X_HEAT_NONE, &temp, &rh));
    teardown();
}

TEST_CASE("sht4x_heat_measure() should return temperature and humidity", "[sht4x]")
{
    uint8_t cmd = SHT4X_CMD_MEASURE;
    const uint8_t data[] = {0x5f, 0x16, 0x1a, 0x5e, 0x35, 0x3b};
    float temp_expected = 20.001144426642256;
    float rh_expected = 40.000228885328454;
    float temp, rh;

    setup();

    read_cb_data = data;
    i2c_master_write_to_device_ExpectAndReturn(PORT, CONFIG_SHT4X_ADDRESS, &cmd,
                                               1, portMAX_DELAY, ESP_OK);
    i2c_master_read_from_device_ExpectAndReturn(PORT, CONFIG_SHT4X_ADDRESS,
                                                NULL, 6, portMAX_DELAY, ESP_OK);
    i2c_master_read_from_device_IgnoreArg_read_buffer();
    i2c_master_read_from_device_AddCallback(read_cb);

    TEST_ASSERT_EQUAL(ESP_OK, sht4x_heat_measure(sht4x, SHT4X_HEAT_NONE, &temp, &rh));
    TEST_ASSERT_FLOAT_WITHIN(DELTA, temp_expected, temp);
    TEST_ASSERT_FLOAT_WITHIN(DELTA, rh_expected, rh);
    teardown();
}

TEST_CASE("sht4x_heat_measure() should not return a relative humidity above 100", "[sht4x]")
{
    uint8_t cmd = SHT4X_CMD_MEASURE;
    const uint8_t data[] = {0x5f, 0x16, 0x1a,
                            0xd9, 0x16, 0x63}; // rh: 0xd916 = 100.000610 %
    float expected = 100.0;
    float temp, rh;

    setup();

    read_cb_data = data;
    i2c_master_write_to_device_ExpectAndReturn(PORT, CONFIG_SHT4X_ADDRESS, &cmd,
                                               1, portMAX_DELAY, ESP_OK);
    i2c_master_read_from_device_ExpectAndReturn(PORT, CONFIG_SHT4X_ADDRESS,
                                                NULL, 6, portMAX_DELAY, ESP_OK);
    i2c_master_read_from_device_IgnoreArg_read_buffer();
    i2c_master_read_from_device_AddCallback(read_cb);

    TEST_ASSERT_EQUAL(ESP_OK, sht4x_heat_measure(sht4x, SHT4X_HEAT_NONE, &temp, &rh));
    TEST_ASSERT_FLOAT_WITHIN(DELTA, expected, rh);
    teardown();
}

TEST_CASE("sht4x_heat_measure() should not return a relative humidity below 0", "[sht4x]")
{
    uint8_t cmd = SHT4X_CMD_MEASURE;
    const uint8_t data[] = {0x5f, 0x16, 0x1a,
                            0x0c, 0x49, 0x80}; // rh: 0x0c49 = -0.001297
    float expected = 0.0;
    float temp, rh;

    setup();

    read_cb_data = data;
    i2c_master_write_to_device_ExpectAndReturn(PORT, CONFIG_SHT4X_ADDRESS, &cmd,
                                               1, portMAX_DELAY, ESP_OK);
    i2c_master_read_from_device_ExpectAndReturn(PORT, CONFIG_SHT4X_ADDRESS,
                                                NULL, 6, portMAX_DELAY, ESP_OK);
    i2c_master_read_from_device_IgnoreArg_read_buffer();
    i2c_master_read_from_device_AddCallback(read_cb);

    TEST_ASSERT_EQUAL(ESP_OK, sht4x_heat_measure(sht4x, SHT4X_HEAT_NONE, &temp, &rh));
    TEST_ASSERT_FLOAT_WITHIN(DELTA, expected, rh);
    teardown();
}

TEST_CASE("sht4x_measure_raw() should handle invalid sensor object", "[sht4x]")
{
    uint32_t temp, rh;

    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, sht4x_measure_raw(sht4x, &temp, &rh));
}

TEST_CASE("sht4x_measure_raw() should return temperature and humidity", "[sht4x]")
{
    uint8_t cmd = SHT4X_CMD_MEASURE;
    const uint8_t data[] = {0x5f, 0x16, 0x1a, 0x5e, 0x35, 0x3b};
    uint32_t temp_expected = 0x5f16;
    uint32_t rh_expected = 0x5e35;
    uint32_t temp, rh;

    setup();

    read_cb_data = data;
    i2c_master_write_to_device_ExpectAndReturn(PORT, CONFIG_SHT4X_ADDRESS, &cmd,
                                               1, portMAX_DELAY, ESP_OK);
    i2c_master_read_from_device_ExpectAndReturn(PORT, CONFIG_SHT4X_ADDRESS,
                                                NULL, 6, portMAX_DELAY, ESP_OK);
    i2c_master_read_from_device_IgnoreArg_read_buffer();
    i2c_master_read_from_device_AddCallback(read_cb);

    TEST_ASSERT_EQUAL(ESP_OK, sht4x_measure_raw(sht4x, &temp, &rh));
    TEST_ASSERT_EQUAL_HEX32(temp_expected, temp);
    TEST_ASSERT_EQUAL_HEX32(rh_expected, rh);
    teardown();
}

TEST_CASE("sht4x_measure() should handle invalid sensor object", "[sht4x]")
{
    float temp, rh;

    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, sht4x_measure(sht4x, &temp, &rh));
}

TEST_CASE("sht4x_measure() should return temperature and humidity", "[sht4x]")
{
    uint8_t cmd = SHT4X_CMD_MEASURE;
    const uint8_t data[] = {0x5f, 0x16, 0x1a, 0x5e, 0x35, 0x3b};
    float temp_expected = 20.001144426642256;
    float rh_expected = 40.000228885328454;
    float temp, rh;

    setup();

    read_cb_data = data;
    i2c_master_write_to_device_ExpectAndReturn(PORT, CONFIG_SHT4X_ADDRESS, &cmd,
                                               1, portMAX_DELAY, ESP_OK);
    i2c_master_read_from_device_ExpectAndReturn(PORT, CONFIG_SHT4X_ADDRESS,
                                                NULL, 6, portMAX_DELAY, ESP_OK);
    i2c_master_read_from_device_IgnoreArg_read_buffer();
    i2c_master_read_from_device_AddCallback(read_cb);

    TEST_ASSERT_EQUAL(ESP_OK, sht4x_measure(sht4x, &temp, &rh));
    TEST_ASSERT_FLOAT_WITHIN(DELTA, temp_expected, temp);
    TEST_ASSERT_FLOAT_WITHIN(DELTA, rh_expected, rh);
    teardown();
}

void test_sht4x(void)
{
    unity_run_tests_by_tag("[sht4x]", false);
}
