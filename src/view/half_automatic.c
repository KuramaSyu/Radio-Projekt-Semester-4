#include <stdio.h>
#include <math.h>

#include "esp_rom_sys.h"

#include "app_state.h"
#include "display.h"
#include "helpers.h"
#include "tea5767.h"
#include "view.h"

static bool float_equals(float a, float b) {
    return fabs(a - b) < 0.01;
}

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
 * Returns a formatted frequency in format "102.4 MHz"
 */
char *get_formatted_frequency(float channel) {
    static char freq_str[16];
    static char freq_str_padded[17]; // 16 chars + null terminator

    snprintf(freq_str, sizeof(freq_str), "%.1f MHz", channel);
    snprintf(freq_str_padded, sizeof(freq_str_padded), "%-16s", freq_str); // pad with spaces to clear old text
    return freq_str_padded;
}

/**
 * Retrieves the channel by slicing the potentiometer range into equally sized parts
 */
float get_channel(int analog_value) {
    float frequencies[] = {89.2, 90.1, 92.2, 95.4, 97.3, 100.2, 102.4, 103.5, 105.2};
    int len = sizeof(frequencies) / sizeof(frequencies[0]);
    float stepsize = 4096 / len;

    int index = (int)(analog_value / stepsize);
    if (index >= len) index = len - 1;

    float channel = frequencies[index];
    return channel;
}

/**
 * @param signal_strength int from 0..15
 *
 * Returns:
 * --------
 * string with format: Signal {gut/ok/schlecht}
 */
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

/**
 * Updates the display and tuner for half automatic mode
 * @param value int from 0..4095
 */
void view_half_automatic_on_pot_change(int value) {
    // get channel
    float channel = get_channel(value);

    // check if channel is the same as before, if not change it
    if (channel == frequency) {
        return;
    }
    frequency = channel;
    tea5767_set_freq(channel);

    // write station name to first row of display
    const char *radio_station_name = get_channel_name(channel);
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print(radio_station_name);

    read_signal_and_write_to_display();
    esp_rom_delay_us(4000000);
    // after 4s show frequency of channel
    lcd_set_cursor(0, 1);
    char *freq_str = get_formatted_frequency(channel);
    write_machine_state_to_last_char(freq_str);
    lcd_print(freq_str);
}
