// CURRENTLY NOT USED; tea5767 iS USED INSTEAD
#ifndef RADIOTUNER_H
#define RADIOTUNER_H

#include "esp_err.h"

/**
 * starts the radio tuner si4703
 */
void si4703_reset(void);

/*
 * Reads the registers of si4703 into `regs`
 */
esp_err_t si4703_read_regs(void);

/*
 * Writes the content of `regs` into the si4703 device
 * by converting 16 bit regs to 8 bit regs and sending these
 *
 * Registers:
 * -----------
 *
 * * 0x00: Device ID
 * * 0x01: Chip ID
 * * 0x02: Power Configuration
 * * 0x03: Channels
 * * 0x05: Bit 3-0 (left to right) Volume
 * * 0x04-0x06: System Configuration
 * * 0x07-0x08: Tests
 * * 0x09: Boot Configuration (reserved)
 * * 0x0A: Status RSSI
 * * 0x0B: Read Channel
 * * 0x0C: RDSA (RDS Block A Data)
 * * 0x0D: RDSB
 * * 0x0E: RDSC
 * * 0x0F: RDSD
 */
esp_err_t si4703_write_regs(void);

/**
 * starts up the chip,
 * sets power up register bits,
 * sets volume (stage 2 of 7)
 */
esp_err_t si4703_init(void);

/**
 * starts up the chip,
 * sets power up register bits,
 * sets volume (stage 2 of 7)
 */
esp_err_t si4703_init2(void);

/**
 * sets the frequency of the radio tuner
 * @param freq_mhz frequency in MHz
 *
 *
 * channel is calculated by subtracting 87.5 MHz from the desired frequency
 * and then multiplying by 10, because the channel spacing is 100 kHz (0.1 MHz). The freq
 * is set in register 0x03. This register also has a TUNE bit, which initiates the process
 */
esp_err_t si4703_set_freq(float freq_mhz);
#endif // RADIOTUNER_H
