#include <math.h>

#include "app_state.h"
#include "display.h"
#include "helpers.h"
#include "tea5767.h"
#include "view.h"

/**
 * Retrieves the frequency directly from the potentiometer value
 * where 87.5 MHz is the first channel and 108 MHz the last
 */
float get_channel_free(int analog_value) {
    // 87.5 MHz is the first channel, 108 MHz the last
    float channel = 87.5 + (analog_value / 4096.0) * (108 - 87.5);
    return channel;
}

/**
 * Updates the display and tuner for manual mode
 * @param value int from 0..4095
 */
void view_manual_on_pot_change(int value) {
    // return if the frequency is same to .1f precision, to avoid unnecessary updates
    if (lroundf(frequency * 10.0f) == lroundf(get_channel_free(value) * 10.0f)) {
        return;
    }
    float channel_manual = get_channel_free(value);
    frequency = channel_manual;
    tea5767_set_freq(channel_manual);

    // show frequency on display
    lcd_clear();
    lcd_set_cursor(0, 0);
    char *freq_str = get_formatted_frequency(channel_manual);
    write_machine_state_to_last_char(freq_str);
    lcd_print(freq_str);
    read_signal_and_write_to_display();
}
