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


bool float_equals(float a, float b) {
    return fabs(a - b) < 0.01;
};

/**
 * Selects the station name based on the channel frequency
 */
const char *get_channel_name(float channel) {
    static char station_name[17]; // 16 chars + null terminator
    int string_size = sizeof(station_name);

    if (float_equals(channel, 89.2)) {snprintf(station_name, string_size, "%-16s","R.SA");}
    else if (float_equals(channel, 90.1)) {snprintf(station_name, string_size, "%-16s","MDR JUMP");}
    else if (float_equals(channel, 92.2)) {snprintf(station_name, string_size, "%-16s","MDR Sachsen");}
    else if (float_equals(channel, 95.4)) {snprintf(station_name, string_size, "%-16s","MDR Kultur");}
    else if (float_equals(channel, 97.3)) {snprintf(station_name, string_size, "%-16s","Deuschlandfunk");}
    else if (float_equals(channel, 100.2)) {snprintf(station_name, string_size, "%-16s","ENERGY Dresden");}
    else if (float_equals(channel, 102.4)) {snprintf(station_name, string_size, "%-16s","Radio PSR");}
    else if (float_equals(channel, 103.5)) {snprintf(station_name, string_size, "%-16s","Radio Dresden");}
    else if (float_equals(channel, 105.2)) {snprintf(station_name, string_size, "%-16s","HitRadio RTL");}
    else {snprintf(station_name, string_size, "%-16s","Unbekannt");}

    return station_name;
}

/**
 * Returns a formatted channel in format "102.4 MHz"
 */
const char *get_formatted_frequency(float channel) {
    static char freq_str[16];
    static char freq_str_padded[17]; // 16 chars + null terminator

    snprintf(freq_str, sizeof(freq_str), "%.1f MHz", channel);
    snprintf(freq_str_padded, sizeof(freq_str_padded), "%-16s", freq_str); // pad with spaces to clear old text
    return freq_str_padded;
}

float get_channel(int analog_value) {
    float frequencies[] = {89.2, 90.1, 92.2, 95.4, 97.3, 100.2, 102.4, 103.5, 105.2};
    int len = sizeof(frequencies) / sizeof(frequencies[0]);
    float stepsize = 4096 / len;

    int index = (int)(analog_value / stepsize);
    if (index >= len) index = len - 1;

    float channel = frequencies[index];
    return channel;
}

float get_channel_free(int analog_value) {
    // 87.5 MHz is the first channel, 108 MHz the last
    float channel = 87.5 + (analog_value / 4096.0) * (108 - 87.5);
    return channel;
}


const char *get_formatted_signal_strength(int signal_strength) {
    static char strength_str[17];
    if (signal_strength >= 11) {
        snprintf(strength_str, sizeof(strength_str), "Signal: sehr gut");
    } else if (signal_strength >= 8) {
        snprintf(strength_str, sizeof(strength_str), "Signal: gut");
    } else if (signal_strength >= 4) {
        snprintf(strength_str, sizeof(strength_str), "Signal: ok");
    } else {
        snprintf(strength_str, sizeof(strength_str), "Signal: schlecht");
    }
    return strength_str;
}

void tea5767_set_freq(float mhz) {
    const char *radio_station_name = get_channel_name(mhz);
    ESP_LOGI(RADIO_TAG, "Change frequency to %.1f HHz (%s)\n", mhz, radio_station_name);
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
            I2C_PORT,
            TEA5767_ADDR,
            buf,
            sizeof(buf),
            pdMS_TO_TICKS(100)
        )
    );
}




