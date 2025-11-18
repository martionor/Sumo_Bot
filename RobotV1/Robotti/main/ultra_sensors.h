#ifndef HC_SR04_H
#define HC_SR04_H

#include "driver/gpio.h"
#include "driver/mcpwm_cap.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// -----------------------------------------------------------------------------
// Pin configuration
// -----------------------------------------------------------------------------
#define HC_SR04_TRIG_GPIO      22
#define HC_SR04_ECHO_GPIO      21

// -----------------------------------------------------------------------------
// MCPWM capture settings
// -----------------------------------------------------------------------------
#define CAPTURE_PRESCALE       1       // Minimum valid value for MCPWM capture
#define HC_SR04_CALIBRATION    0.9f    // Calibration factor for measured distance

// -----------------------------------------------------------------------------
// Public API
// -----------------------------------------------------------------------------

/**
 * @brief Initialize MCPWM capture and Trig pin
 */
void hc_sr04_setup(void);

/**
 * @brief Generate a single 10 µs pulse on the Trig pin
 */
void hc_sr04_gen_trig_pulse(void);

/**
 * @brief Measure distance in cm using MCPWM capture timer
 * @return Distance in cm, or negative for errors:
 *         -1: Timeout waiting for echo
 *         -2: Pulse out of range
 */
float hc_sr04_measure_distance_cm(void);

#endif // HC_SR04_H
