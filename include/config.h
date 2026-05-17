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


// TEA5767 radio tuner I2C pins
#define TEA5767_I2C_SDA_PIN 8
#define TEA5767_I2C_SCL_PIN 9

// Potentiometer ADC mapping
#define POTENTIOMETER_ADC_UNIT ADC_UNIT_1
#define POTENTIOMETER_ADC_CHANNEL ADC1_CHANNEL_6

// OLD RADIO TUNER - UNUSED: SI4703 tuner control pins (I2C + reset)
#define SI4703_I2C_SDA_PIN 13
#define SI4703_I2C_SCL_PIN 14
#define SI4703_RST_PIN 12

#endif