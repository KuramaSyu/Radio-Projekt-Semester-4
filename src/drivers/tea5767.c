#include <stdio.h>

#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"

#include "config.h"

static const char *RADIO_TAG = "radio";

void i2c_init(void) {
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = TEA5767_I2C_SDA_PIN,
        .scl_io_num = TEA5767_I2C_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = RADIO_I2C_SPEED_HZ,
    };

    ESP_ERROR_CHECK(i2c_param_config(RADIO_I2C_PORT, &i2c_config));
    ESP_ERROR_CHECK(i2c_driver_install(RADIO_I2C_PORT, i2c_config.mode, 0, 0, 0));
}

void tea5767_set_freq(float mhz) {
    ESP_LOGI(RADIO_TAG, "Change frequency to %.1f HHz\n", mhz);
    uint8_t buf[5] = {0};

    // datasheet: https://cdn.sparkfun.com/assets/4/5/f/a/d/TEA5767.pdf

    // pll controlls frequency for oscillator
    // 4*(f_rf + f_if) / f_ref = N -> formula for highside injection -> comes from Datasheet
    // 4*(1XX MHz + 225 KHz) / 32668 = factor N for oscillator
    // this factor N is written into the pll register as hex
    uint32_t pll = (4 * ((mhz * 1000000) + 225000)) / 32768;

    // Datasheet: programmable divider (N) data byte 1 = XX11 1111 (0x3F)
    // -> first 2 bits are not used for pll
    // bit 8: mute on/off -> 0
    // bit 7: search on/off -> 0
    // bit 6..1: higher 6 bits of pll
    buf[0] = (pll >> 8) & 0x3F; // ignore first 2 bits -> hence mask with & 0011 1111

    // bit 8..1: lower 8 bits of pll
    buf[1] = pll & 0xFF; // &1111 1111 to ensure only 8 bits are used

    // Datasheet page 13/40
    // 0xB0 = 1011 0000
    // bit 8: 1 = search up, 0 = search down -> does not matter, search is off
    // bit 7 and 6: search stop level -> does not matter
    // bit 5: 1 = high side injection, 0 = low side injection -> high side injection
    // bit 4: mono to stereo -> does not matter
    // bit 3: mute right
    // bit 2: mute left
    // bit 1: software programmable port (??)
    buf[2] = 0xB0;

    // 0x10 = 0001 0000
    // bit 5: clock frequency. 1 = 32.768 kHz
    // bit 2: stereo noise cancelling
    buf[3] = 0x10;

    // bit 8 and 7 are other options
    // bit 6..1 -> unused
    buf[4] = 0x00;

    ESP_ERROR_CHECK(
        i2c_master_write_to_device(
            RADIO_I2C_PORT,
            TEA5767_I2C_ADDR,
            buf,
            sizeof(buf),
            pdMS_TO_TICKS(100)
        )
    );
}
