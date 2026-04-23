#include "driver/i2c.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define RST_PIN 12
#define I2C_ADDR 0x10
#define I2C_MASTER_NUM I2C_NUM_0
#define SDA_PIN 13
#define SCL_PIN 14

#define TAG "radio(si4703)"

void si4703_reset();

/**
 * starts the radio tuner si4703
 */
void si4703_reset() {
    ESP_LOGI(TAG, "Resetting SI4703 on GPIO%d", RST_PIN);
    gpio_set_level(SDA_PIN, 0);
    ESP_LOGI(TAG, "Configuring reset pin GPIO%d", RST_PIN);
    gpio_set_direction(RST_PIN, GPIO_MODE_OUTPUT);
    vTaskDelay(pdMS_TO_TICKS(10));

    // set resset pin for a short duration to low, to start the tuner
    ESP_LOGI(TAG, "Pulling reset LOW");
    gpio_set_level(RST_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    
    ESP_LOGI(TAG, "Pulling reset HIGH");
    gpio_set_level(RST_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(100));  // Give chip time to stabilize after reset
    ESP_LOGI(TAG, "SI4703 reset complete");
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
    esp_err_t ret = i2c_master_write_to_device(
        I2C_MASTER_NUM,
        I2C_ADDR,
        data,
        12,
        pdMS_TO_TICKS(100)
    );
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write registers: %d", ret);
    }
    return ret;
}

/**
 * Scan I2C bus for responding devices (debug helper)
 */
void i2c_scanner() {
    ESP_LOGI(TAG, "Starting I2C bus scan on I2C_NUM_%d at 100kHz", I2C_MASTER_NUM);
    uint8_t address;
    esp_err_t ackerr;
    int found_count = 0;
    for (address = 1; address < 127; address++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        ackerr = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(50));
        i2c_cmd_link_delete(cmd);
        if (ackerr == ESP_OK) {
            ESP_LOGI(TAG, "Found I2C device at address 0x%02X", address);
            found_count++;
        }
    }
    ESP_LOGI(TAG, "I2C scan complete, found %d device(s)", found_count);
}

/**
 * starts up the chip,
 * sets power up register bits,
 * sets volume (stage 2 of 7)
 */
esp_err_t si4703_init() {
    ESP_LOGI(TAG, "Initializing SI4703 at I2C address 0x%02X", I2C_ADDR);
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

/**
 * starts up the chip,
 * sets power up register bits,
 * sets volume (stage 2 of 7)
 */
esp_err_t si4703_init2() {
    // https://github.com/sparkfun/Si4703_FM_Tuner_Evaluation_Board/blob/master/Firmware/Si4703_Example/Si4703_Example.ino#L536-L573
    gpio_set_direction(RST_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(SDA_PIN, GPIO_MODE_OUTPUT);

    gpio_set_level(SDA_PIN, 0); // SDIO to LOW
    gpio_set_level(RST_PIN, 0); // RST to LOW
    vTaskDelay(pdMS_TO_TICKS(1));

    gpio_set_level(RST_PIN, 1); // RST to HIGH
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