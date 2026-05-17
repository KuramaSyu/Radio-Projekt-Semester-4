#include "esp_attr.h"

#include "app_state.h"

// interrupt service routine for button press
void IRAM_ATTR button_isr_handler(void *arg) {
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
