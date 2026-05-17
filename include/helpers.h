#ifndef HELPERS_H
#define HELPERS_H

/**
 * Takes in a char pointer and writes the mode:
 * * "FREI" for manual
 * * "AUTO" for auto
 * to the last char of the string, so that it can be displayed on the LCD
 */
void write_machine_state_to_last_char(char *str);

/**
 * reads the signal strength from the tea5767 and writes it to the second row
 * of the display. If signal strength cannot be read, "n/a" is shown
 */
void read_signal_and_write_to_display(void);

/**
 * Scan I2C bus for responding devices (debug helper)
 */
void i2c_scanner(void);

#endif
