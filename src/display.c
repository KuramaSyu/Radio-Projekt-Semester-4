#include "driver/i2c.h"
#include "esp_log.h"

esp_err_t lcd_write_byte(uint8_t b) {
    return i2c_master_write_to_device(
        I2C_NUM_1, 
        0x27, // default address from datasheet is either 0x3F or 0x27
        &b,
        1,
        1000 / portTICK_PERIOD_MS
    );
};

/**
 * Enable display   
 */
void lcd_pulse_enable(uint8_t data) {
    lcd_write_byte(data | 0x04); // 0x04 = enable display -> set 1 at position 4
    esp_rom_delay_us(1);
    lcd_write_byte(data & ~0x04); // remove the 0x04 which was set before
    esp_rom_delay_us(50);
};

/**
 * writes data and sets display backlight on
 * 
 * parameters:
 * * control: sets the set bit via or. 1 mean data; 0 means command
 */
void lcd_write_nibble(uint8_t nibble, uint8_t control) {
    uint8_t data = (
        (nibble << 4) // move data into frist 4 register bits
        | control // set control bit at bit 0 -> control = 0 means its data
        | 0x08 // set 4th bit to 1 to enable backlight
    );
    lcd_write_byte(data);
    lcd_pulse_enable(data);
}

/**
 * Sets a bit by setting 2 nibbles
 * 
 * parameters:
 * -----------
 * 
 * * value:
 * * control (bool): whether or not to set control bit (0 = data, 1 = control)
 */
void lcd_send_byte(uint8_t value, uint8_t control) {
    uint8_t control_bit = control ? 0x01 : 0x00; // set control bit (first bit)

    lcd_write_nibble(value >> 4, control_bit);  // first 4 characters of byte
    lcd_write_nibble(value & 0x0F, control_bit); // last 4 characters of byte
};

void lcd_command(uint8_t cmd) {
    lcd_send_byte(cmd, 0);
};

void lcd_data(uint8_t data) {
    lcd_send_byte(data, 1);
}

void lcd_init() {
    // wait ~50ms after mC turned on, to let lcd reset internally
    vTaskDelay(50 / portTICK_PERIOD_MS);

    // write 3 times 0011, which is the reset sequence. It's written 3 times, because
    // display could be in different states
    lcd_write_nibble(0x03, 0);  // command
    vTaskDelay(5 / portTICK_PERIOD_MS);

    lcd_write_nibble(0x03, 0);  // command
    esp_rom_delay_us(150);

    lcd_write_nibble(0x03, 0);

    // switch to 4-bit mode
    lcd_write_nibble(0x02, 0);  


    lcd_command(0x28);  // function set (binary 0010 1000) 2 lines, 5x8 font
    lcd_command(0x0C);  // 1100 -> set display command,turn display on, cursor off, blinking off
    lcd_command(0x08); // display off
    lcd_command(0x01);  // clear
    esp_rom_delay_us(2000);
    lcd_command(0x06);  // set entry mode
    lcd_command(0x0C); // display on
    vTaskDelay(5 / portTICK_PERIOD_MS);
    return;
}

/**
 * höchst räudig; sollte verboten sein; ist es auch in Rust
 */
void lcd_print(const char *str) {
    while (*str) {
        lcd_data(*str++);
    }
}

/**
 * parameters
 * ----------
 * 
 * * col (int): which column (0-15)?
 * * row (int): which row (0-1)?
 */
void lcd_set_cursor(uint8_t col, uint8_t row) {
    uint8_t addr = (row == 0) ? 0x00 : 0x40;  // set 0100 0000 if row is 1, otherwise 0000 0000
    lcd_command(0x80 | (addr + col)); // 0x80 (1000 0000 -> 1st / 8th bit is to set to data command)
};