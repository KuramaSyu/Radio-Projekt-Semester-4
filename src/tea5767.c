#include <stdio.h>
#include <math.h>

#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"

#define I2C_SDA 8
#define I2C_SCL 9
#define I2C_PORT I2C_NUM_0
#define I2C_SPEED_HZ 100000

#define TEA5767_ADDR 0x60

static const char *RADIO_TAG = "radio";

void i2c_init(void) {
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA,
        .scl_io_num = I2C_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_SPEED_HZ,
    };

    ESP_ERROR_CHECK(i2c_param_config(I2C_PORT, &i2c_config));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_PORT, i2c_config.mode, 0, 0, 0));
}

void tea5767_set_freq(float mhz) {
    const char *radio_station_name = get_channel_name(mhz);
    ESP_LOGI(RADIO_TAG, "Change frequency to %.1f HHz (%s)\n", mhz, radio_station_name);
    uint8_t buf[5] = {0};

    uint32_t pll = (4 * ((mhz * 1000000) + 225000)) / 32768;

    buf[0] = (pll >> 8) & 0x3F;//?
    buf[1] = pll & 0xFF; //?

    // Search mode off
    // High side injection
    buf[2] = 0xB0;
    
    // Stereo ON
    buf[3] = 0x10;

    buf[4] = 0x00;

    ESP_ERROR_CHECK(
        i2c_master_write_to_device(
            I2C_PORT,
            TEA5767_ADDR,
            buf,
            sizeof(buf),
            pdMS_TO_TICKS(100)
        )
    );
}

float get_channel(int analog_value) {
    float frequencies[] = {89.2, 100.2, 102.4, 103.5, 105.2, 106.1};
    int len = sizeof(frequencies) / sizeof(frequencies[0]);
    float stepsize = 4096 / len;
    int index = (int)(analog_value / stepsize);
    float channel = frequencies[index];
    return channel;
}

const char *get_channel_name(float channel) {
    static char station_name[32];
    snprintf(station_name, sizeof(station_name), "%.1f MHz", channel);
    return station_name;
}
