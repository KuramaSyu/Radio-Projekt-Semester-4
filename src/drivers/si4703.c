// CURRENTLY NOT USED BECAUSE I2C STARTUP SEQUENCE WAS NOT WORKING
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "config.h"

#define I2C_MASTER_NUM RADIO_I2C_PORT

#define TAG "radio(si4703)"

void si4703_reset();

void si4703_reset() {
    ESP_LOGI(TAG, "Resetting SI4703 on GPIO%d", SI4703_RST_PIN);
    gpio_set_level(SI4703_I2C_SDA_PIN, 0);
    ESP_LOGI(TAG, "Configuring reset pin GPIO%d", SI4703_RST_PIN);
    gpio_set_direction(SI4703_RST_PIN, GPIO_MODE_OUTPUT);
    vTaskDelay(pdMS_TO_TICKS(10));

    // set resset pin for a short duration to low, to start the tuner
    ESP_LOGI(TAG, "Pulling reset LOW");
    gpio_set_level(SI4703_RST_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    
    ESP_LOGI(TAG, "Pulling reset HIGH");
    gpio_set_level(SI4703_RST_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(100));  // Give chip time to stabilize after reset
    ESP_LOGI(TAG, "SI4703 reset complete");
}

// radio tuner has 16 16Bit Registers
uint16_t regs[16];

esp_err_t si4703_read_regs() {
    // reading is done in 8Bit
    uint8_t data[32];

    // access device
    esp_err_t ret = i2c_master_read_from_device(
        I2C_MASTER_NUM,
        SI4703_I2C_ADDR,
        data,
        32,
        pdMS_TO_TICKS(100)
    );

    // device not accessible
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read registers: %d", ret);
        return ret;
    }

    for (int i = 0; i < 16; i++ ) {
        // compose a 16Bit Register out of 2 8Bit Register
        regs[i] = (data[2*i] << 8) | data[2*i + 1];
    }

    return ESP_OK;
}

esp_err_t si4703_write_regs() {
    ESP_LOGD(TAG, "Writing to si4703");
    uint8_t data[12]; // data for 6 registers from 0x02 - 0x07

    // convert 16 bit registers to 8 bit registers
    for (int i = 0; i < 6; i++) {
        // read first (left to right) 8 bit (hence move the 16bit to right)
        data[2*i] = regs[2+i] >> 8; 
        // read last (left to right) 8 bit (hence mask it with 0000 0000 1111 1111)
        data[2*i + 1] = regs[2+i] & 0xFF; 
    }

    // access device
    esp_err_t ret = i2c_master_write_to_device(
        I2C_MASTER_NUM,
        SI4703_I2C_ADDR,
        data,
        12,
        pdMS_TO_TICKS(100)
    );
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write registers: %d", ret);
    }
    return ret;
}

esp_err_t si4703_init() {
    ESP_LOGI(TAG, "Initializing SI4703 at I2C address 0x%02X", SI4703_I2C_ADDR);
    si4703_reset();
    
    // Scan I2C bus to debug
    //i2c_scanner();
    
    ESP_LOGI(TAG, "Reading SI4703 registers after reset");
    si4703_read_regs();

    // power up and update regs
    regs[0x02] = 0x4001; // from datasheet
    ESP_LOGI(TAG, "Writing power config: 0x%04X", regs[0x02]);
    si4703_write_regs();
    vTaskDelay(pdMS_TO_TICKS(110));
    ESP_LOGI(TAG, "Reading SI4703 registers after power-up");
    si4703_read_regs();

    // volume (Bit 3-0) + unmute bit (Bit 6)
    // 0x0040 = unmute enable, 0x000F = max volume
    regs[0x05] = 0x004F;  // unmuted, max volume
    ESP_LOGI(TAG, "Writing volume/unmute: 0x%04X", regs[0x05]);
    si4703_write_regs();

    return ESP_OK;
}

esp_err_t si4703_init2() {
    // https://github.com/sparkfun/Si4703_FM_Tuner_Evaluation_Board/blob/master/Firmware/Si4703_Example/Si4703_Example.ino#L536-L573
    gpio_set_direction(SI4703_RST_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(SI4703_I2C_SDA_PIN, GPIO_MODE_OUTPUT);

    gpio_set_level(SI4703_I2C_SDA_PIN, 0); // SDIO to LOW
    gpio_set_level(SI4703_RST_PIN, 0); // RST to LOW
    vTaskDelay(pdMS_TO_TICKS(1));

    gpio_set_level(SI4703_RST_PIN, 1); // RST to HIGH
    vTaskDelay(pdMS_TO_TICKS(1));

    si4703_read_regs();
    regs[0x07] = 0x8100; // enable oscillator
    si4703_write_regs();
    vTaskDelay(pdMS_TO_TICKS(500));

    si4703_read_regs();
    regs[0x02] = 0x4001;  // configure power
    regs[0x04] |= (1<<12); // enable RDS
    regs[0x04] |= (1<<11); // 50kHz Europe setup
    regs[0x05] |= (1<<4);  // 100kHz channel spacing Europe
    regs[0x05] &= 0xFFF0;  // clear volume
    regs[0x05] |= 0x0001; // set volume
    vTaskDelay(pdMS_TO_TICKS(110)); //  max powerup time, from datesheet page 13
    return ESP_OK;
}

esp_err_t si4703_set_freq(float freq_mhz) {
    ESP_LOGI(TAG, "Setting frequency to %.1f MHz", freq_mhz);
    ESP_LOGD(TAG, "Reading registers before tuning");
    si4703_read_regs();

    // 87.5 is first channel, 108 the last
    // example: 102.4 MHz -> 102.4 - 87.5 = 14.9 -> 149 / 0x95 / 1001 0101
    int channel = (freq_mhz - 87.5) * 10;

    regs[0x03] &= ~(0x03FF); // clear channel by masking with 1111 1100 0000 0000
    regs[0x03] |= channel; // set channel by applying first 8 bits
    regs[0x03] |= 0x8000; // set TUNE Bit to HIGH

    // when tune bit is enabled, the chip
    // initiates the tuning process, by applying the 
    // configured values from the registers
    ESP_LOGI(TAG, "Starting tune request for channel %d", channel);
    si4703_write_regs();
    vTaskDelay(pdMS_TO_TICKS(60));

    // remove TUNE Bit, otherwise changing the freq again would fail
    regs[0x03] &= ~(0x8000); // set TUNE Bit to LOW
    ESP_LOGD(TAG, "Clearing tune bit after tune request");
    si4703_write_regs();

    return ESP_OK;
}