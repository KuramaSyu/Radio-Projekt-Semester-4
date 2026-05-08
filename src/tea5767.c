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




