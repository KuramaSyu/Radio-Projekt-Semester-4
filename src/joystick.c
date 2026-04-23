#include "esp_log.h"
#include "esp_adc_cal.h"
#include "driver/adc.h"
#include "joystick.h"


static const char *TAG = "joystick";

#define JOY_ADC_CHANNEL_X ADC_CHANNEL_0  // should be same channel as configured for the pin
#define JOY_ADC_CHANNEL_Y ADC_CHANNEL_1

#define ADC_ATTEN ADC_ATTEN_DB_11
#define ADC_WIDTH ADC_WIDTH_BIT_12
#define DEFAULT_VREF 1100 // ESP32 S3 reference voltage is at about mV

/* Thresholds for mapping axis to left/center/right */
#define ADC_LOW_THRESHOLD 1400
#define ADC_HIGH_TRESHOLD 2800

// calibration types
typedef struct {
    int center;
    int min; 
    int max; 
    int low_thr;
    int high_thr;
    int deadzone;
} axis_cal_t;

static axis_cal_t cal_x = {0};
static axis_cal_t cal_y = {0};

static esp_adc_cal_characteristics_t adc_chars;

void setup_joystick() {
    adc1_config_width(ADC_WIDTH);
    adc1_config_channel_atten(JOY_ADC_CHANNEL_X, ADC_ATTEN);
    adc1_config_channel_atten(JOY_ADC_CHANNEL_Y, ADC_ATTEN);
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN, ADC_WIDTH, DEFAULT_VREF, &adc_chars);
    ESP_LOGI(TAG, "Joystick ADC initialized");

}

/* Diagnostic: print ADC1 channels 0..7 raw values once. Move joystick and observe which channel changes. */
void joystick_adc_scan_once(void)
{
    for (int ch = 0; ch <= 7; ++ch) {
        int raw = adc1_get_raw((adc_channel_t)ch);
        ESP_LOGI(TAG, "ADC1 CH%d raw=%d", ch, raw);
    }
}

/* read raw adc values (12 bit -> 0..4095) */
void joystick_read_raw(int *x_raw, int *y_raw) {
    if (x_raw) *x_raw = adc1_get_raw(JOY_ADC_CHANNEL_X);
    if (y_raw) *y_raw = adc1_get_raw(JOY_ADC_CHANNEL_Y);
}

/* helper to take samples and calculate average */
static int adc_avg_samples(adc_channel_t ch, int samples, int delay_ms) {
    long sum = 0;
    for (int i = 0; i < samples; ++i) {
        sum += adc1_get_raw(ch);
        esp_rom_delay_us(delay_ms * 1000);
    }
    return (int)(sum / samples);
}

/* calibare the joystick interactivly via logs */
void joystick_calibrate(void) {
    const int samples = 30; 
    ESP_LOGI(TAG, "Calibrate: leave joystick at CENTER");
    esp_rom_delay_us(2000000);
    cal_x.center = adc_avg_samples(JOY_ADC_CHANNEL_X, samples, 20);
    cal_y.center = adc_avg_samples(JOY_ADC_CHANNEL_Y, samples, 20);
    ESP_LOGI(TAG, "Center X=%d  Y=%d", cal_x.center, cal_y.center);

    ESP_LOGI(TAG, "Calibrate: move joystick to LEFT (hold)");
    esp_rom_delay_us(1500000);
    cal_x.min = adc_avg_samples(JOY_ADC_CHANNEL_X, samples, 20);
    ESP_LOGI(TAG, "Left X=%d", cal_x.min);

    ESP_LOGI(TAG, "Calibrate: move joystick to RIGHT (hold)");
    esp_rom_delay_us(1500000);
    cal_x.max = adc_avg_samples(JOY_ADC_CHANNEL_X, samples, 20);
    ESP_LOGI(TAG, "Right X=%d", cal_x.max);

    ESP_LOGI(TAG, "Calibrate: move joystick to UP (hold)");
    esp_rom_delay_us(1500000);
    cal_y.max = adc_avg_samples(JOY_ADC_CHANNEL_Y, samples, 20);
    ESP_LOGI(TAG, "Up Y=%d", cal_y.max);

    ESP_LOGI(TAG, "Calibrate: move joystick to DOWN (hold)");
    esp_rom_delay_us(1500000);
    cal_y.min = adc_avg_samples(JOY_ADC_CHANNEL_Y, samples, 20);
    ESP_LOGI(TAG, "Down Y=%d", cal_y.min);

    /* get thredholds */
    cal_x.low_thr = (cal_x.center + cal_x.min) / 2;
    cal_x.high_thr = (cal_x.center + cal_x.max) / 2;
    cal_y.low_thr = (cal_y.center + cal_y.min) / 2;
    cal_y.high_thr = (cal_y.center + cal_y.max) / 2;

    /* deadzone (inner 4th of the circle) */
    int span_x = (cal_x.center - cal_x.min < cal_x.max - cal_x.center) ? (cal_x.center - cal_x.min) : (cal_x.max - cal_x.center);
    int span_y = (cal_y.center - cal_y.min < cal_y.max - cal_y.center) ? (cal_y.center - cal_y.min) : (cal_y.max - cal_y.center);
    cal_x.deadzone = span_x / 4;
    cal_y.deadzone = span_y / 4;

    ESP_LOGI(TAG, "X thr low=%d center=%d high=%d dead=%d", cal_x.low_thr, cal_x.center, cal_x.high_thr, cal_x.deadzone);
    ESP_LOGI(TAG, "Y thr low=%d center=%d high=%d dead=%d", cal_y.low_thr, cal_y.center, cal_y.high_thr, cal_y.deadzone);
}

/** map using calibrated values
* @return int -1, 0 if in deadzone, 1
*/
int joystick_map_axis_calibrated(int raw, const axis_cal_t *c) {
    int diff = raw - c->center; 
    if (abs(diff) <= c->deadzone) return 0;
    if (diff < 0) {
        return (raw <= c->low_thr) ? -1 : 0;
    } else {
        return (raw >= c->high_thr) ? 1 : 0;
    }
}

void joystick_get_states_calibrated(int *x_state, int *y_state) {
    int xr = adc1_get_raw(JOY_ADC_CHANNEL_X);
    int yr = adc1_get_raw(JOY_ADC_CHANNEL_Y);
    if (x_state) *x_state = joystick_map_axis_calibrated(xr, &cal_x);
    if (y_state) *y_state = joystick_map_axis_calibrated(yr, &cal_y);
}