#include "ultra_sensors.h"
#include "esp_log.h"
#include "esp_private/esp_clk.h"

// Internal state variables
static volatile uint32_t start_tick = 0;
static volatile uint32_t end_tick = 0;
static volatile uint32_t duration_ticks = 0;

// MCPWM capture ISR
static bool hc_sr04_capture_cb(mcpwm_cap_channel_handle_t cap_chan,
                               const mcpwm_capture_event_data_t *edata,
                               void *user_data)
{
    TaskHandle_t task_to_notify = (TaskHandle_t)user_data;
    BaseType_t task_woken = pdFALSE;

    if (edata->cap_edge == MCPWM_CAP_EDGE_POS) {
        start_tick = edata->cap_value;
    } else {
        end_tick = edata->cap_value;
        duration_ticks = end_tick - start_tick;

        xTaskNotifyFromISR(task_to_notify, 1, eSetBits, &task_woken);
    }

    return task_woken == pdTRUE;
}

// Trigger pulse generator
void hc_sr04_gen_trig_pulse(void)
{
    gpio_set_level(HC_SR04_TRIG_GPIO, 1);
    esp_rom_delay_us(10);
    gpio_set_level(HC_SR04_TRIG_GPIO, 0);
}

// Measure distance in cm
float hc_sr04_measure_distance_cm(void)
{
    const float tick_to_us = (float)((CAPTURE_PRESCALE + 1) / 40e6f * 1e6f) * HC_SR04_CALIBRATION;

    hc_sr04_gen_trig_pulse();

    uint32_t notified;
    if (xTaskNotifyWait(0, ULONG_MAX, &notified, pdMS_TO_TICKS(1000)) != pdTRUE) {
        return -1.0f; // timeout
    }

    float pulse_us = duration_ticks * tick_to_us;

    if (pulse_us > 35000.0f) return -2.0f;

    return pulse_us / 58.0f;
}

// Setup MCPWM capture and Trig pin
void hc_sr04_setup(void)
{
    // Trig pin
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << HC_SR04_TRIG_GPIO,
    };
    gpio_config(&io_conf);
    gpio_set_level(HC_SR04_TRIG_GPIO, 0);

    // MCPWM timer
    mcpwm_cap_timer_handle_t cap_timer;
    mcpwm_capture_timer_config_t cap_timer_conf = {
        .clk_src = MCPWM_CAPTURE_CLK_SRC_XTAL,
        .group_id = 0,
    };
    ESP_ERROR_CHECK(mcpwm_new_capture_timer(&cap_timer_conf, &cap_timer));
    ESP_ERROR_CHECK(mcpwm_capture_timer_enable(cap_timer));
    ESP_ERROR_CHECK(mcpwm_capture_timer_start(cap_timer));

    // MCPWM channel
    mcpwm_cap_channel_handle_t cap_chan;
    mcpwm_capture_channel_config_t cap_chan_conf = {
        .gpio_num = HC_SR04_ECHO_GPIO,
        .prescale = CAPTURE_PRESCALE,
        .flags.pos_edge = true,
        .flags.neg_edge = true,
        .flags.pull_up = true,
    };
    ESP_ERROR_CHECK(mcpwm_new_capture_channel(cap_timer, &cap_chan_conf, &cap_chan));

    // Register ISR callback
    TaskHandle_t task = xTaskGetCurrentTaskHandle();
    mcpwm_capture_event_callbacks_t cbs = { .on_cap = hc_sr04_capture_cb };
    ESP_ERROR_CHECK(mcpwm_capture_channel_register_event_callbacks(cap_chan, &cbs, task));
    ESP_ERROR_CHECK(mcpwm_capture_channel_enable(cap_chan));
}
