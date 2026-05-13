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

// both are used to check if 
// values are changed compared to previous value
float frequency = 0;
int pot_raw_value = 0;

// timer function which is set up to be
// periodically called
// since internupt is not possible with 
// analog device. But potentiometer
// is better user experience then buttons
static void pot_timer_task(void *arg) {
    int now = read_pot_raw();
    if (abs(pot_raw_value - now) > 80) {
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
    // get channel
    float channel = get_channel(value);

    // check if channel is the same as before, if not change it
    if (channel == frequency) {
        return;
    }
    frequency = channel;
    tea5767_set_freq(channel);

    // write station name to first row of display
    const char *radio_station_name = get_channel_name(channel);
    lcd_set_cursor(0, 0);
    lcd_print(radio_station_name);

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
        esp_rom_delay_us(4000000);
    } else {
        lcd_set_cursor(0, 1);
        lcd_print("Signal: n/a");
        esp_rom_delay_us(4000000);
    }
    // after 4s show frequency of channel
    lcd_set_cursor(0, 1);
    lcd_print(get_formatted_frequency(channel));


}

void app_main() {

    esp_rom_delay_us(5000000);  // 5s wait, to ensure, that I see all logs. otherwise that was not the case
    ESP_LOGI(TAG, "Program start");

    
    // connect to display
    i2c_display_init();

    // setup fm radio unit (tea5767)
    i2c_init();
    
    // init potentiometer
    adc_init();

    // init LCD Display (set cursor, set lines, set light etc)
    // (needs to be initialized after pot timer start
    // since the potentiometer timer updates the display on freq changes)
    ESP_LOGI(TAG, "init lcd");
    lcd_init(); // weird sequence of commands

    // setup timer to periodically check it's analog value (100ms)
    const esp_timer_create_args_t args = {
        .callback = &pot_timer_task,
        .name = "pot_timer",
    };
    esp_timer_handle_t pot_timer;
    ESP_ERROR_CHECK(esp_timer_create(&args, &pot_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(pot_timer, 100000));
};

