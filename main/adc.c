#include "adc.h"

#include "driver/adc.h"
#include "esp_adc_cal.h"

#define ADC_SAMPLING_NUM 32

static const char * TAG = "Dulab ADC";
static adc_t adc = {
    {0,},
};

static void adc_task(void* arg)
{
    int adc6_buf, adc7_buf;

    for (;;) {
        adc6_buf = 0;
        adc7_buf = 0;
        for (int i = 0; i < ADC_SAMPLING_NUM; i++) {
            adc6_buf += adc1_get_raw(ADC1_CHANNEL_6);
            adc7_buf += adc1_get_raw(ADC1_CHANNEL_7);
        }
        adc.reading[0] = adc6_buf / ADC_SAMPLING_NUM;
        adc.reading[1] = adc7_buf / ADC_SAMPLING_NUM;
        
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

adc_t * adc_init(void)
{
    ESP_LOGI(TAG, "[APP] %s", __func__);

    adc1_config_width(ADC_WIDTH_BIT_DEFAULT);
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_2_5);
    adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_2_5);

    xTaskCreate(adc_task, "gpio_task_example", 2048, NULL, 10, NULL);

    return &adc;
}
