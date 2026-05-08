#include <stdio.h>
#include <math.h>

#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"

/**
 * Args:
 * -----
 * * * value: int from 0..15
 * 
 * Returns:
 * --------
 * string with format: Signal {gut/ok/schlecht}
 */
const char *get_formatted_signal_strength(int signal_strength);

/**
 * Returns a formatted frequency in format "102.4 MHz"
 */
const char *get_formatted_frequency(float channel);

/**
 * checks if 2 floats are about equal
 */
bool float_equals(float a, float b);

/**
 * Returns a formatted channel label for the given frequency
 */
const char *get_channel_name(float channel);

/**
 * initializes the tea5767 radio unit
 */
void i2c_init(void);

/**
 * writes the frequency into the register of the tea5767 unit
 */
void tea5767_set_freq(float mhz);

/**
 * Retrieves the channel by slicing the pot range into equally sized parts
 */
float get_channel(int analog_value);

