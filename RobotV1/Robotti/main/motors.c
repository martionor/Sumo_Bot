#include "motor.h"
#include "driver/gpio.h"

#define SERVO_MIN_US   1000  // full reverse
#define SERVO_STOP_US  1500  // stop
#define SERVO_MAX_US   2000  // full forward

#define PWM_FREQUENCY  50    // 50 Hz = standard servo
#define PWM_RESOLUTION LEDC_TIMER_16_BIT

#define LEFT_MOTOR_GPIO   18
#define RIGHT_MOTOR_GPIO  19

static const int motor_gpio[2] = {
    LEFT_MOTOR_GPIO,
    RIGHT_MOTOR_GPIO
};

esp_err_t motor_init(void)
{
    // Timer config
    ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = PWM_RESOLUTION,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = PWM_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer));

    // Channel config for each motor
    for (int i = 0; i < 2; i++) {
        ledc_channel_config_t channel = {
            .gpio_num = motor_gpio[i],
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel = i,          // channel 0 = left, 1 = right
            .timer_sel = LEDC_TIMER_0,
            .duty = 0,
            .hpoint = 0
        };
        ESP_ERROR_CHECK(ledc_channel_config(&channel));
    }

    return ESP_OK;
}

/**
 * Convert microseconds to LEDC duty
 */
static uint32_t us_to_duty(int us)
{
    const int max_duty = (1 << PWM_RESOLUTION) - 1;
    const float period_us = 1e6f / PWM_FREQUENCY;
    return (uint32_t)((us / period_us) * max_duty);
}

esp_err_t motor_set_speed(motor_id_t motor, int speed)
{
    if (motor < 0 || motor > 1) return ESP_ERR_INVALID_ARG;
    if (speed < -100) speed = -100;
    if (speed > 100) speed = 100;

    // map -100..100 → 1000..2000 us
    int pulse = SERVO_STOP_US + (SERVO_MAX_US - SERVO_STOP_US) * speed / 100;

    uint32_t duty = us_to_duty(pulse);

    return ledc_set_duty(LEDC_LOW_SPEED_MODE, motor, duty) |
           ledc_update_duty(LEDC_LOW_SPEED_MODE, motor);
}
