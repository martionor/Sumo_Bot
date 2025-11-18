#ifndef MOTOR_H
#define MOTOR_H

#include "driver/ledc.h"
#include "esp_err.h"

typedef enum {
    MOTOR_LEFT = 0,
    MOTOR_RIGHT = 1
} motor_id_t;

/**
 * @brief Initialize the motor subsystem
 */
esp_err_t motor_init(void);

/**
 * @brief Set motor speed
 *
 * @param motor MOTOR_LEFT or MOTOR_RIGHT
 * @param speed Range: -100 to +100
 *              -100 = full reverse
 *               0   = stop
 *             +100 = full forward
 */
esp_err_t motor_set_speed(motor_id_t motor, int speed);

#endif // MOTOR_H
