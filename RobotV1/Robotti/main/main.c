#include <stdio.h>
#include "motor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{
    motor_init();

    while (1) {
        motor_set_speed(MOTOR_LEFT, 50);
        motor_set_speed(MOTOR_RIGHT, 50);
        vTaskDelay(pdMS_TO_TICKS(1500));

        motor_set_speed(MOTOR_LEFT, -50);
        motor_set_speed(MOTOR_RIGHT, -50);
        vTaskDelay(pdMS_TO_TICKS(1500));

        motor_set_speed(MOTOR_LEFT, 0);
        motor_set_speed(MOTOR_RIGHT, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
