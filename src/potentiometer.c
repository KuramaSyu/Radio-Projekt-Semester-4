#include "driver/adc.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

// Handle for the ADC oneshot unit
// oneshot = not continuos
static adc_oneshot_unit_handle_t adc_handle;

// handle for the ADC calibration (convert raw to mV)
static adc_cali_handle_t cali_handle;

void adc_init(void) {
    adc_oneshot_unit_init_cfg_t init_cfg = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE, // disables ultra-low-power co-processor sampling but use CPU
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_cfg, &adc_handle));

    adc_oneshot_chan_cfg_t chan_cfg = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_11 // Docs: ADC_ATTEN_DB_11 150 mV ~ 2450 mV
    };

    // ADC1 Channel 6 is on PIN 7
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ADC1_CHANNEL_6, &chan_cfg));

    // calibration of adc
    // https://docs.espressif.com/projects/esp-idf/en/v5.0/esp32c3/api-reference/peripherals/adc_calibration.html
    adc_cali_curve_fitting_config_t cali_cfg = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_11,
        .bitwidth = ADC_BITWIDTH_12
    };
    ESP_ERROR_CHECK(adc_cali_create_scheme_curve_fitting(&cali_cfg, &cali_handle));
}


// returns value 0 - 4095
int read_pot_raw(void) {
    int raw = 0;
    ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ADC1_CHANNEL_6, &raw));

    return raw; 
};
