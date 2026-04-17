#include "driver/i2c.h"
#include "esp_log.h"

#define RST_PIN 12
#define I2C_ADDR 0x10
#define I2C_MASTER_NUM I2C_NUM_0

#define TAG "radio(si4703)"

void si4703_reset();

/**
 * starts the radio tuner si4073
 */
void si4703_reset() {
    gpio_set_direction(RST_PIN, GPIO_MODE_OUTPUT);

    // set resset pin for a short duration to low, to start the tuner
    gpio_set_level(RST_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(RST_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(10));
}

// radio tuner has 16 16Bit Registers
uint16_t regs[16];

/*
 * Reads the registers of si4703 into `regs` 
 */
esp_err_t si4703_read_regs() {
    // reading is done in 8Bit
    uint8_t data[32];

    // access device
    esp_err_t ret = i2c_master_read_from_device(
        I2C_MASTER_NUM,
        I2C_ADDR,
        data,
        32,
        pdMS_TO_TICKS(100)
    );

    // device not accessible
    if (ret != ESP_OK) return ret;

    for (int i = 0; i < 16; i++ ) {
        // compose a 16Bit Register out of 2 8Bit Register
        regs[i] = (data[2*i] << 8) | data[2*i + 1];
    }

    return ESP_OK;
}

/*
 * Writes the content of `regs` into the si4703 device
 * by converting 16 bit regs to 8 bit regs and sending these
 * 
 * Registers:
 * -----------
 * 
 * * 0x00: Device ID
 * * 0x01: Chip ID
 * * 0x02: Power Configuration
 * * 0x03: Channels
 * * 0x05: Bit 3-0 (left to right) Volume
 * * 0x04-0x06: System Configuration
 * * 0x07-0x08: Tests
 * * 0x09: Boot Configuration (reserved)
 * * 0x0A: Status RSSI
 * * 0x0B: Read Channel
 * * 0x0C: RDSA (RDS Block A Data)
 * * 0x0D: RDSB
 * * 0x0E: RDSC
 * * 0x0F: RDSD
 */
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
    esp_err_t ret = i2c_master_read_from_device(
        I2C_MASTER_NUM,
        I2C_ADDR,
        data,
        12,
        pdMS_TO_TICKS(100)
    );
    return ESP_OK;
}

/**
 * starts up the chip,
 * sets power up register bits,
 * sets volume (stage 2 of 7)
 */
esp_err_t si4703_init() {
    ESP_LOGD(TAG, "Resetting si4703");
    si4703_reset();
    si4703_read_regs();

    // power up and update regs
    regs[0x02] = 0x4001; // from datasheet
    si4703_write_regs();
    vTaskDelay(pdMS_TO_TICKS(110));
    si4703_read_regs();

    // volume (Bit 3 - 0) and unmute
    regs[0x05] = 0x0010; // volume
    si4703_write_regs();

    return ESP_OK;
}

/**
 * sets the frequency of the radio tuner
 * @param freq_mhz frequency in MHz
 * 
 * 
 * channel is calculated by subtracting 87.5 MHz from the desired frequency 
 * and then multiplying by 10, because the channel spacing is 100 kHz (0.1 MHz). The freq
 * is set in register 0x03. This register also has a TUNE bit, which initiates the process
 */
esp_err_t si4703_set_freq(float freq_mhz) {
    ESP_LOGI(TAG, "Setting frequency to %.1f MHz", freq_mhz);
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
    si4703_write_regs();
    vTaskDelay(pdMS_TO_TICKS(60));

    // remove TUNE Bit, otherwise changing the freq again would fail
    regs[0x03] &= ~(0x8000); // set TUNE Bit to LOW
    si4703_write_regs();

    return ESP_OK;
}