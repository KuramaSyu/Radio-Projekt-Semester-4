#include "driver/i2c.h"
#include "esp_log.h"

#include "app_state.h"
#include "display.h"
#include "view.h"

#define I2C_MASTER_NUM I2C_NUM_0

/**
 * Takes in a char pointer and writes the mode:
 * * "FREI" for manual
 * * "AUTO" for auto
 * to the last char of the string, so that it can be displayed on the LCD
 */
void write_machine_state_to_last_char(char *str) {

    if (machine_state == STATE_MANUAL) {
        str[12] = 'F';
        str[13] = 'R';
        str[14] = 'E';
        str[15] = 'I';
    } else if (machine_state == STATE_HALF_AUTO) {

        str[12] = 'A';
        str[13] = 'U';
        str[14] = 'T';
        str[15] = 'O';
    } else {
        str[15] = '?';
    }
}

/**
 * reads the signal strength from the tea5767 and writes it to the second row
 * of the display. If signal strength cannot be read, "n/a" is shown
 */
void read_signal_and_write_to_display() {
    // read signal strength from tea5767
    uint8_t status[5];
    if (i2c_master_read_from_device(
        I2C_NUM_0,
        0x60, // tea5767 address
        status,
        5,
        pdMS_TO_TICKS(100)
    ) == ESP_OK) {
        // show signal strength for 4s on second row of display
        int signal = status[3] >> 4;
        lcd_set_cursor(0, 1);
        lcd_print(get_formatted_signal_strength(signal));
        // show signal strength for 4s
    } else {
        lcd_clear();
        lcd_set_cursor(0, 1);
        lcd_print("Signal: n/a");
    }
}

/**
 * Scan I2C bus for responding devices (debug helper)
 */
void i2c_scanner() {
    ESP_LOGI("radio(si4703)", "Starting I2C bus scan on I2C_NUM_%d at 100kHz", I2C_MASTER_NUM);
    uint8_t address;
    esp_err_t ackerr;
    int found_count = 0;
    for (address = 1; address < 127; address++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        ackerr = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(50));
        i2c_cmd_link_delete(cmd);
        if (ackerr == ESP_OK) {
            ESP_LOGI("radio(si4703)", "Found I2C device at address 0x%02X", address);
            found_count++;
        }
    }
    ESP_LOGI("radio(si4703)", "I2C scan complete, found %d device(s)", found_count);
}
