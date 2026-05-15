#include "display.h"
#include "radiotuner.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "tea5767.h"
#include "potentiometer.h"
#include "esp_timer.h"
#include "button.h"
#include "config.h"

///////////////// DECLARATIONS //////////////////
static void pot_timer_task(void *arg);
static void on_pot_change(int value);
void write_machine_state_to_last_char(char *str);
void read_signal_and_write_to_display();

//////////////// CONSTANTS AND VARIABLES /////////////////
static const char *TAG = "main";
static const char *TIMER_TAG = "timer";
#define JOYSTICK_READ_X 11
#define JOYSTICK_READ_Y 10 

// create enum to define machine state (manual freq or freq from list)
typedef enum {
    STATE_MANUAL, // using potentiometer to select frequency freely
    STATE_HALF_AUTO, // using potentiometer to select from list of channels
} machine_state_t;

volatile machine_state_t machine_state = STATE_HALF_AUTO;





// both are used to check if 
// values are changed compared to previous value
float frequency = 0;
int pot_raw_value = 0;
machine_state_t last_machine_state = STATE_HALF_AUTO;


////////////// IMPLEMENTATIONS //////////////////

// interrupt service routine for button press
void IRAM_ATTR button_isr_handler(void* arg) {
    // debounce is not implemented,
    // since there were no debouncing issues during testing
    switch (machine_state)
    {
    case STATE_MANUAL:
        machine_state = STATE_HALF_AUTO;
        break;
    case STATE_HALF_AUTO:
        machine_state = STATE_MANUAL;
        break;
    default:
        break;
    }
}


/** 
 * A Timer which gets called from ESP-IDF.
 * It checks if the potentiometer value has changed enough 
 * OR if the machine state has changed. Then it calls on_pot_change
 * which is used to update frequency and hence update display.
 * 
* It's periodically called
* since internupt is not possible with 
* analog device. But potentiometer
* is better user experience then buttons
*/ 
static void pot_timer_task(void *arg) {
    int now = read_pot_raw();
    int potentiometer_change_threshold;
    switch (machine_state)
    {
    case STATE_HALF_AUTO:
        potentiometer_change_threshold = 80;
        break;
    case STATE_MANUAL:
        potentiometer_change_threshold = 35;
        break;
    default:
        potentiometer_change_threshold = 80;
        break;
    }

    

    if (
        abs(pot_raw_value - now) > potentiometer_change_threshold  // big enough change
        || machine_state != last_machine_state  // OR state change
    ) {
        // pot change is not just noice/jitter -> call event
        ESP_LOGI(TAG, "Dispatch potentiometer change (v=%d)", now);
        on_pot_change(now);
        pot_raw_value = now;
        last_machine_state = machine_state;
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
    ESP_LOGI(TIMER_TAG, "Potentiometer value: %d; Update frequency and display", value);
    const char *freq_str;
    
    switch (machine_state)
    {
    case STATE_HALF_AUTO:
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
        lcd_clear();
        lcd_set_cursor(0, 0);
        lcd_print(radio_station_name);

        read_signal_and_write_to_display();
        esp_rom_delay_us(4000000);
        // after 4s show frequency of channel
        lcd_set_cursor(0, 1);
        freq_str = get_formatted_frequency(channel);
        write_machine_state_to_last_char(freq_str);
        lcd_print(freq_str);
        break;

    case STATE_MANUAL:
        // return if the frequency is same to .1f precision, to avoid unnecessary updates
        if (lroundf(frequency * 10.0f) == lroundf(get_channel_free(value) * 10.0f)) {
            return;
        }
        float channel_manual = get_channel_free(value);
        frequency = channel_manual;
        tea5767_set_freq(channel_manual);

        // show frequency on display
        lcd_clear();
        lcd_set_cursor(0, 0);
        const char *freq_str = get_formatted_frequency(channel_manual);
        write_machine_state_to_last_char(freq_str);
        lcd_print(freq_str);
        read_signal_and_write_to_display();
        break;
    
    default:
        break;
    }
    


}

void app_main() {

    esp_rom_delay_us(5000000);  // 5s wait, to ensure, that I see all logs. otherwise that was not the case
    ESP_LOGI(TAG, "Program start");
    ESP_LOGI(TAG, "Register button press service routine");
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_PIN, button_isr_handler, NULL);
    
    // connect to display
    i2c_display_init();

    // setup fm radio unit (tea5767)
    i2c_init();
    
    // init potentiometer
    adc_init();

    // init button
    button_init();

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