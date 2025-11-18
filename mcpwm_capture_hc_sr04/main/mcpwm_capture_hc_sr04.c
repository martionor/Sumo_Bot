#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/mcpwm_cap.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_private/esp_clk.h"

#define TAG "HC_SR04"
#define HC_SR04_TRIG_GPIO  22
#define HC_SR04_ECHO_GPIO  21
#define CAPTURE_PRESCALE   1   // MCPWM minimum prescale
#define HC_SR04_CALIBRATION 0.9f

static volatile uint32_t start_tick = 0;
static volatile uint32_t end_tick   = 0;
static volatile uint32_t duration_ticks = 0;

// MCPWM capture ISR — works like AVR TIMER1_CAPT_vect
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

        // notify the task
        xTaskNotifyFromISR(task_to_notify, 1, eSetBits, &task_woken);
    }

    return task_woken == pdTRUE;
}

// Trigger pulse generator
static void gen_trig_pulse(void)
{
    gpio_set_level(HC_SR04_TRIG_GPIO, 1);
    esp_rom_delay_us(10);
    gpio_set_level(HC_SR04_TRIG_GPIO, 0);
}

// Measure distance in cm using MCPWM ticks
float measure_distance_cm(void)
{
     const float tick_to_us = (float)((CAPTURE_PRESCALE + 1) / 40e6f * 1e6f)*HC_SR04_CALIBRATION;

    // Trigger sensor
    gen_trig_pulse();

    // Wait for ISR to notify
    uint32_t notified;
    if (xTaskNotifyWait(0, ULONG_MAX, &notified, pdMS_TO_TICKS(1000)) != pdTRUE) {
        return -1.0f; // timeout
    }

    // Convert ticks → microseconds
    float pulse_us = duration_ticks * tick_to_us;

    // Out of range check
    if (pulse_us > 35000.0f) return -2.0f;

    // Convert to cm (HC-SR04: 58 us per cm)
    return pulse_us / 58.0f;
}

// Setup MCPWM capture and Trig pin
static void hc_sr04_setup(void)
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

void app_main(void)
{
    hc_sr04_setup();

    while (1) {
        float dist = measure_distance_cm();

        if (dist > 0) {
            ESP_LOGI(TAG, "Distance: %.2f cm", dist);
        } else if (dist == -1) {
            ESP_LOGW(TAG, "Timeout waiting for echo");
        } else if (dist == -2) {
            ESP_LOGW(TAG, "Out of range");
        }

        vTaskDelay(pdMS_TO_TICKS(500)); // trigger every 500 ms
    }
}
