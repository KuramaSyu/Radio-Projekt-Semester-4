#ifndef VIEW_H
#define VIEW_H

/**
 * Returns a formatted frequency in format "102.4 MHz"
 */
char *get_formatted_frequency(float channel);

/**
 * @param signal_strength int from 0..15
 *
 * Returns:
 * --------
 * string with format: Signal {gut/ok/schlecht}
 */
const char *get_formatted_signal_strength(int signal_strength);

/**
 * Updates the display and tuner for manual mode
 * @param value int from 0..4095
 */
void view_manual_on_pot_change(int value);

/**
 * Updates the display and tuner for half automatic mode
 * @param value int from 0..4095
 */
void view_half_automatic_on_pot_change(int value);

/**
 * Retrieves the frequency directly from the potentiometer value
 * where 87.5 MHz is the first channel and 108 MHz the last
 */
float get_channel_free(int analog_value);

/**
 * Retrieves the channel by slicing the potentiometer range into equally sized parts
 */
float get_channel(int analog_value);

/**
 * Selects the station name based on the channel frequency
 */
const char *get_channel_name(float channel);

#endif
