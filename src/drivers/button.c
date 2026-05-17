#include "esp_log.h"
#include "driver/gpio.h"
#include "stdio.h"
#include "config.h"

void button_init() {
    gpio_config_t button_conf = {
        .pin_bit_mask = 1ULL << BUTTON_PIN,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE, // trigger on falling edge (button press)
    };
    gpio_config(&button_conf);

}
