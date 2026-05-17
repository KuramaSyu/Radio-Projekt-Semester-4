#include "display.h"
#include "esp_log.h"
#include "tea5767.h"
#include "potentiometer.h"
#include "esp_timer.h"
#include "button.h"
#include "config.h"
#include "interrupts.h"
#include "timers.h"

//////////////// CONSTANTS AND VARIABLES /////////////////
static const char *TAG = "main";

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
    // (needs to be initialized after potentiometer timer start
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
