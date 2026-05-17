#ifndef APP_STATE_H
#define APP_STATE_H

// create enum to define machine state (manual freq or freq from list)
typedef enum {
    STATE_MANUAL, // using potentiometer to select frequency freely
    STATE_HALF_AUTO, // using potentiometer to select from list of channels
} machine_state_t;

extern volatile machine_state_t machine_state;
extern float frequency;
extern int pot_raw_value;
extern machine_state_t last_machine_state;

#endif
