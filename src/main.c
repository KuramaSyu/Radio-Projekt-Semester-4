#include "display.h"
#include "radiotuner.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "joystick.h"



static const char *TAG = "example";
#define JOYSTICK_READ_X 11
#define JOYSTICK_READ_Y 10

void app_main() {

    esp_rom_delay_us(5000000);
    ESP_LOGI(TAG, "Program start");

    // i2c is currently connected with PIN 13 = SDA and PIN 14 SCL
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = 13,
        .scl_io_num = 14,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 1000,
    };

    // configure I2C
    ESP_LOGI(TAG, "configure i2c");
    i2c_param_config(I2C_NUM_0, &i2c_config);
    i2c_driver_install(I2C_NUM_0, i2c_config.mode, 0, 0, 0);

    // configure radio reset pin
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << 12),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    // configure controller pins as input
    // gpio_config_t joystick_x_conf = {
    //     .pin_bit_mask = (1ULL << JOYSTICK_READ_X),
    //     .mode = GPIO_MODE_INPUT,
    //     .pull_up_en = GPIO_PULLUP_ENABLE,
    //     .pull_down_en = GPIO_PULLDOWN_DISABLE,
    //     .intr_type = GPIO_INTR_DISABLE
    // };
    // gpio_config(&joystick_x_conf);
    //     gpio_config_t joystick_y_conf = {
    //     .pin_bit_mask = (1ULL << JOYSTICK_READ_Y),
    //     .mode = GPIO_MODE_INPUT,
    //     .pull_up_en = GPIO_PULLUP_ENABLE,
    //     .pull_down_en = GPIO_PULLDOWN_DISABLE,
    //     .intr_type = GPIO_INTR_DISABLE
    // };
    // gpio_config(&joystick_y_conf);
    // setup_joystick();
    // joystick_calibrate();
    // int state_x = 0;
    // int state_y = 0;

    si4703_init2();
    esp_rom_delay_us(100000);
    si4703_set_freq(102.4);

    // ESP_LOGI(TAG, "init lcd");
    // lcd_init(); // weird sequence of commands

    // ESP_LOGI(TAG, "set cursor");
    // lcd_set_cursor(0, 0);
    // ESP_LOGI(TAG, "write hello");
    // lcd_print("Helloooo");

    while (1) {
        // esp_rom_delay_us(1000000);
        // ESP_LOGI(TAG, "reinit radio");
        // si4703_init2();
        ESP_LOGI(TAG, "Scanning for i2c");
        i2c_scanner();
        // int joystick_x = gpio_get_level(JOYSTICK_READ_X);
        // joystick_get_states_calibrated(&state_x, &state_y);
        // ESP_LOGI(TAG, "X: %d; Y: %d", state_x, state_y);

        si4703_read_regs();
        ESP_LOGI(TAG, "In while loop");
    }
};

