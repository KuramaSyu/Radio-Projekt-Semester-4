#ifndef TIMERS_H
#define TIMERS_H

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
void pot_timer_task(void *arg);

#endif
