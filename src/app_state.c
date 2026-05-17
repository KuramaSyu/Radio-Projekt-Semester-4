#include "app_state.h"

volatile machine_state_t machine_state = STATE_HALF_AUTO;

// both are used to check if
// values are changed compared to previous value
float frequency = 0;
int pot_raw_value = 0;
machine_state_t last_machine_state = STATE_HALF_AUTO;
