#include "display.h"
#include "radiotuner.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "tea5767.h"
#include "potentiometer.h"
#include "esp_timer.h"




static const char *TAG = "main";
static const char *TIMER_TAG = "timer";
#define JOYSTICK_READ_X 11
#define JOYSTICK_READ_Y 10

static void pot_timer_task(void *arg);
static void on_pot_change(int value);

int pot_raw_value = 0;

// timer function which is set up to be
// periodically called
// since internupt is not possible with 
// analog device. But potentiometer
// is better user experience then buttons
static void pot_timer_task(void *arg) {
    int now = read_pot_raw();
    if (abs(pot_raw_value - now) > 100) {
        // pot change is not just noice/jitter -> call event
        ESP_LOGI(TAG, "Dispatch potentiometer change (v=%d)", now);
        on_pot_change(now);
        pot_raw_value = now;
    };
}


/**
 * check channel. If channel is not the same,
 * then chagne it
 * 
 * Args:
 * -----
 * * value: int from 0..4095
 */
static void on_pot_change(int value) {
    float channel = get_channel(value);
    tea5767_set_freq(channel);
}

void app_main() {

    esp_rom_delay_us(5000000);
    ESP_LOGI(TAG, "Program start");

    // i2c is currently connected with PIN 13 = SDA and PIN 14 SCL
    // i2c_config_t i2c_config = {
    //     .mode = I2C_MODE_MASTER,
    //     .sda_io_num = 13,
    //     .scl_io_num = 14,
    //     .sda_pullup_en = GPIO_PULLUP_ENABLE,
    //     .scl_pullup_en = GPIO_PULLUP_ENABLE,
    //     .master.clk_speed = 1000,
    // };

    // // configure I2C
    // ESP_LOGI(TAG, "configure i2c");
    // i2c_param_config(I2C_NUM_0, &i2c_config);
    // i2c_driver_install(I2C_NUM_0, i2c_config.mode, 0, 0, 0);

    // // configure radio reset pin
    // gpio_config_t io_conf = {
    //     .pin_bit_mask = (1ULL << 12),
    //     .mode = GPIO_MODE_INPUT,
    //     .pull_up_en = GPIO_PULLUP_ENABLE,
    //     .pull_down_en = GPIO_PULLDOWN_DISABLE,
    //     .intr_type = GPIO_INTR_DISABLE
    // };
    // gpio_config(&io_conf);

    // setup fm radio unit (tea5767)
    i2c_init();
    tea5767_set_freq(102.4);
   


    // init potentiometer
    adc_init();

    // setup timer to periodically check it's analog value (100ms)
    const esp_timer_create_args_t args = {
        .callback = &pot_timer_task,
        .name = "pot_timer",
    };
    esp_timer_handle_t pot_timer;
    ESP_ERROR_CHECK(esp_timer_create(&args, &pot_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(pot_timer, 100000));


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

    // si4703_init2();
    // esp_rom_delay_us(100000);
    // si4703_set_freq(102.4);

    // ESP_LOGI(TAG, "init lcd");
    // lcd_init(); // weird sequence of commands

    // ESP_LOGI(TAG, "set cursor");
    // lcd_set_cursor(0, 0);
    // ESP_LOGI(TAG, "write hello");
    // lcd_print("Helloooo");

    // while (1) {
    //     // esp_rom_delay_us(1000000);
    //     // ESP_LOGI(TAG, "reinit radio");
    //     // si4703_init2();
    //     // ESP_LOGI(TAG, "Scanning for i2c");
    //     // i2c_scanner();
        
    //     // uint8_t status[5];

    //     // if (i2c_master_read_from_device(
    //     //     I2C_NUM_0,
    //     //     TEA5767_ADDR,
    //     //     status,
    //     //     5,
    //     //     pdMS_TO_TICKS(100)
    //     // ) == ESP_OK) {
    //     //     int stereo = (status[2] & 0x80) ? 1 : 0;
    //     //     int signal = status[3] >> 4;

    //     //     ESP_LOGI(TAG, "Stereo=%d, Signal=%d/15", stereo, signal);

    //     // }
    // }
};

