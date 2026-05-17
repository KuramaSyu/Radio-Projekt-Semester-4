/**
 * global config file for PINs. This file was added late, hence many
 * statics are missing here
 */

#ifndef CONFIG_H
#define CONFIG_H

// Button input pin
#define BUTTON_PIN 18

// LCD display I2C pins
#define DISPLAY_I2C_SDA_PIN 13
#define DISPLAY_I2C_SCL_PIN 14
// LCD display I2C bus and address
#define DISPLAY_I2C_PORT I2C_NUM_1
#define DISPLAY_I2C_ADDR 0x27

// Shared I2C settings for radio tuners
#define RADIO_I2C_PORT I2C_NUM_0
#define RADIO_I2C_SPEED_HZ 100000


// TEA5767 radio tuner I2C pins
#define TEA5767_I2C_SDA_PIN 8
#define TEA5767_I2C_SCL_PIN 9
// TEA5767 radio tuner I2C address
#define TEA5767_I2C_ADDR 0x60

// Potentiometer ADC mapping
#define POTENTIOMETER_ADC_UNIT ADC_UNIT_1
#define POTENTIOMETER_ADC_CHANNEL ADC1_CHANNEL_6

// OLD RADIO TUNER - UNUSED: SI4703 tuner control pins (I2C + reset)
#define SI4703_I2C_SDA_PIN 13
#define SI4703_I2C_SCL_PIN 14
#define SI4703_RST_PIN 12
// OLD RADIO TUNER - UNUSED: SI4703 I2C address
#define SI4703_I2C_ADDR 0x10

#endif