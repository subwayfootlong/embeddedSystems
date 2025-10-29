#include "mq2_driver.h"
#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

/**
 * @brief Internal driver state
 */
static bool is_initialized = false;
static Mq2Config config;
static absolute_time_t warmup_deadline;
static absolute_time_t last_sample_time;

int mq2_init(const Mq2Config *cfg)
{
    if (cfg == NULL ||
        cfg->min_interval_ms < 1000 ||
        cfg->adc_channel > 3) {
        return MQ2_ERR_INVAL;
    }

    config = *cfg;

    adc_init();
    adc_gpio_init(26 + config.adc_channel);
    adc_select_input(config.adc_channel);

    last_sample_time = make_timeout_time_ms(0);
    is_initialized = true;

    return MQ2_OK;
}

int mq2_warmup()
{
    if (!is_initialized) {
        return MQ2_ERR_NO_INIT;
    }

    warmup_deadline = make_timeout_time_ms(config.warmup_ms);

    return MQ2_OK;
}

bool mq2_ready()
{
    if (!is_initialized) {
        return false;
    }

    return time_reached(warmup_deadline);
}

int mq2_sample(float *ppm_out, float *voltage_out)
{
    if (!is_initialized) {
        return MQ2_ERR_NO_INIT;
    }

    if (!mq2_ready()) {
        return MQ2_ERR_BUSY;
    }

    absolute_time_t now = get_absolute_time();
    int64_t elapsed_ms = absolute_time_diff_us(last_sample_time, now) / 1000;

    if (elapsed_ms < (int64_t)config.min_interval_ms) {
        return MQ2_ERR_BUSY;
    }

    uint16_t raw = adc_read();
    if (raw == 0 || raw >= 4095) {
        return MQ2_ERR_HW;
    }

    const float kAdcScale = VCC_ADC_VOLTAGE / 4096.0f;
    float voltage = raw * kAdcScale;

    float rs_ohm = ((VCC_ADC_VOLTAGE / voltage) - 1.0f) * RL_OHM;
    float rs_ro_ratio = rs_ohm / R0_CLEAN_AIR_OHM;
    
    if (rs_ro_ratio > 0) {
        float log10_rs_ro = log10(rs_ro_ratio);
        float log10_ppm = (log10_rs_ro - LPG_INTERCEPT) / LPG_SLOPE;
        
        *ppm_out = pow(10.0f, log10_ppm);
    } else {
        *ppm_out = 0.0f; 
    }

    if (voltage_out) {
        *voltage_out = voltage;
    }
    
    last_sample_time = now;

    return MQ2_OK;
}