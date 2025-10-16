/*

Parallax continous rotation servo


*/


#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define SERVO_PIN 18

void set_servo_speed(float duty_percent)
{
    // duty_percent: 0-100, but we use around 5–10 for servo pulses
    uint32_t max_duty = (1 << 16) - 1; // 16-bit resolution
    uint32_t duty = (uint32_t)((duty_percent / 100.0) * max_duty);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void app_main(void)
{
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_16_BIT,
        .freq_hz = 50, // 50Hz for servo
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = SERVO_PIN,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&ledc_channel);

    while (1) {
        // Stop
        set_servo_speed(7.6);
        vTaskDelay(pdMS_TO_TICKS(2000));

        // Rotate forward
        set_servo_speed(9.0);
        vTaskDelay(pdMS_TO_TICKS(2000));

        // Stop again
        set_servo_speed(7.5);
        vTaskDelay(pdMS_TO_TICKS(1000));

        // Rotate backward
        set_servo_speed(6.0);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
