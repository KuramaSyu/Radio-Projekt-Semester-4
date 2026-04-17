#include "driver/i2c.h"
#include "esp_log.h"

void lcd_pulse_enable(uint8_t data);
esp_err_t lcd_write_byte(uint8_t b);
void lcd_pulse_enable(uint8_t data);
void lcd_send_byte(uint8_t value, uint8_t control);
void lcd_command(uint8_t cmd);
void lcd_data(uint8_t data);
void lcd_init();
void lcd_print(const char *str);
void lcd_set_cursor(uint8_t col, uint8_t row);