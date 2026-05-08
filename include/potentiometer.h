/**
 * initializes the potentiometer by configuring 
 * an analog digital converter
 */
void adc_init(void);

/**
 * reads the raw analog value from the potentiometer
 *
 * Returns:
 * --------
 * * int: 0..4095
 */
int read_pot_raw(void);