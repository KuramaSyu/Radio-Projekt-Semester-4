#ifndef APP_TEA5767_H
#define APP_TEA5767_H

/**
 * initializes the tea5767 radio unit
 */
void i2c_init(void);

/**
 * writes the frequency into the register of the tea5767 unit
 */
void tea5767_set_freq(float mhz);

#endif
