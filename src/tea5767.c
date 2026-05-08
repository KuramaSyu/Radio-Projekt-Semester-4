#include <stdio.h>
#include <math.h>

#include "driver/i2c.h"
#include "esp_err.h"

#define I2C_SDA 8
#define I2C_SCL 9
#define I2C_PORT I2C_NUM_0
#define I2C_SPEED_HZ 100000

#define TEA5767_ADDR 0x60

static void i2c_init(void) {
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

static void tea5767_set_freq(float mhz) {
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
