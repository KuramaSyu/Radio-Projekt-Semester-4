#include <stdlib.h>

#include "esp_log.h"

#include "app_state.h"
#include "potentiometer.h"
#include "view.h"

static const char *TAG = "main";
static const char *TIMER_TAG = "timer";

static void on_pot_change(int value);

/**
 * A Timer which gets called from ESP-IDF.
 * It checks if the potentiometer value has changed enough
 * OR if the machine state has changed. Then it calls on_pot_change
 * which is used to update frequency and hence update display.
 *
 * It's periodically called
 * since interrupt is not possible with
 * analog device. But potentiometer
 * is better user experience then buttons
 */
void pot_timer_task(void *arg) {
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
        // potentiometer change is not just noice/jitter -> call event
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

    switch (machine_state)
    {
    case STATE_HALF_AUTO:
        view_half_automatic_on_pot_change(value);
        break;
    case STATE_MANUAL:
        view_manual_on_pot_change(value);
        break;
    default:
        break;
    }
}
