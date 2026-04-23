#ifndef INCLUDE_JOYSTICK_H
#define INCLUDE_JOYSTICK_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialize ADC for joystick reads
void setup_joystick(void);

// Read raw ADC values (12-bit: 0..4095)
void joystick_read_raw(int *x_raw, int *y_raw);

// Read voltages in millivolts using esp_adc_cal
void joystick_read_mv(uint32_t *x_mv, uint32_t *y_mv);

// Interactive calibration routine (logs prompts)
void joystick_calibrate(void);

// Get calibrated axis states: -1 (left/up), 0 (center), +1 (right/down)
void joystick_get_states_calibrated(int *x_state, int *y_state);

// Diagnostic: print ADC1 channels raw values once (useful to find correct channel)
void joystick_adc_scan_once(void);

#ifdef __cplusplus
}
#endif

#endif // INCLUDE_JOYSTICK_H

