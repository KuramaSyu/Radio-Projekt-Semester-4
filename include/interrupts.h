#ifndef INTERRUPTS_H
#define INTERRUPTS_H

// interrupt service routine for button press
void IRAM_ATTR button_isr_handler(void *arg);

#endif
