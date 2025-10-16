
/*

    Code for Parallax QTI infrared sensor

*/

#include "driver/adc.h"
#include "hal/adc_types.h"
#include "hal/adc_ll.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/adc_types.h"
#include "hal/adc_ll.h"
#include "esp_log.h"
#include "driver/gpio.h"

#define TAG "QTI_SENSOR"
#define QTI_ADC_CHANNEL ADC1_CHANNEL_3  // GPIO3

void app_main(void){
     // 12-bit ADC resolution
    adc1_config_width(ADC_WIDTH_BIT_12);

    // 0–3.3V range
    adc1_config_channel_atten(QTI_ADC_CHANNEL, ADC_ATTEN_DB_11);

    while(1) {
        int val = adc1_get_raw(QTI_ADC_CHANNEL);
        ESP_LOGI(TAG, "ADC Value: %d", val);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
