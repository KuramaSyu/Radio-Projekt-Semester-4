#ifndef APP_DISPLAY_H
#define APP_DISPLAY_H

#include "driver/i2c.h"
#include "esp_log.h"

/**
 * Enable display
 */
void lcd_pulse_enable(uint8_t data);

/**
 * Writes a single byte to the LCD over I2C
 */
esp_err_t lcd_write_byte(uint8_t b);
void lcd_pulse_enable(uint8_t data);

/**
 * Sets a bit by setting 2 nibbles
 *
 * @param value
 * @param control whether or not to set control bit (0 = data, 1 = control)
 */
void lcd_send_byte(uint8_t value, uint8_t control);

/**
 * Sends a command byte to the LCD
 */
void lcd_command(uint8_t cmd);

/**
 * Sends a data byte to the LCD
 */
void lcd_data(uint8_t data);

/**
 * Initializes the LCD in 4-bit mode
 */
void lcd_init();

/**
 * hoechst raeudig; sollte verboten sein; ist es auch in Rust
 */
void lcd_print(const char *str);

/**
 * parameters
 *
 * @param col which column (0-15)?
 * @param row which row (0-1)?
 */
void lcd_set_cursor(uint8_t col, uint8_t row);

/**
 * Initializes the I2C bus for the LCD display
 */
void i2c_display_init();

/**
 * clears the display
 */
void lcd_clear();

#endif