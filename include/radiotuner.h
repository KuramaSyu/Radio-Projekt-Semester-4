#ifndef RADIOTUNER_H
#define RADIOTUNER_H

#include "esp_err.h"

void si4703_reset(void);
esp_err_t si4703_read_regs(void);
esp_err_t si4703_write_regs(void);
esp_err_t si4703_init(void);
esp_err_t si4703_set_freq(float freq_mhz);

#endif // RADIOTUNER_H
