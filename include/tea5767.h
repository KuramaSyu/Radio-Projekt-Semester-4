#include <stdio.h>
#include <math.h>

#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"

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

/**
 * Returns a formatted channel label for the given frequency
 */
const char *get_channel_name(float channel);