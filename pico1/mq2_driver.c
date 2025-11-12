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
static mq2_config config;
static absolute_time_t warmup_deadline;
static absolute_time_t last_sample_time;

int mq2_init(const mq2_config *cfg)
{
    if (cfg == NULL ||
        cfg->min_interval_ms < 1000 ||
        cfg->adc_channel > 3) {
        return MQ2_ERR_INVAL;
    }

    config = *cfg;

    adc_init();
    adc_gpio_init(MQ2_ADC_GPIO + config.adc_channel);
    adc_select_input(config.adc_channel);

    last_sample_time = make_timeout_time_ms(0);
    is_initialized = true;

    return MQ2_OK;
}

mq2_config mq2_get_config() {
    return config;
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
    if (raw == 0 || raw > 4095) {
        return MQ2_ERR_HW;
    }
    
    float voltage = raw * (V_REF / kAdcMax);

    float rs_ohm = ((VCC_ADC_VOLTAGE / voltage) - 1.0f) * RL_OHM;
    float rs_ro_ratio = rs_ohm / R0_CLEAN_AIR_OHM;

    float current_ppm = 0.0f;

    if (rs_ro_ratio > 0.0f) {
        float log10_rs_ro = log10f(rs_ro_ratio);
        float log10_ppm = (log10_rs_ro - LPG_INTERCEPT) / LPG_SLOPE;
        current_ppm = pow(10.0f, log10_ppm);
    }

    /* Exponential Moving Average (EMA) filter with clamping */
    {
        static float filtered_ppm = 0.0f;
        const float max_ppm_voltage = current_ppm;

        filtered_ppm = EMA_ALPHA * current_ppm + (1.0f - EMA_ALPHA) * filtered_ppm;

        if (filtered_ppm > max_ppm_voltage) {
            filtered_ppm = max_ppm_voltage;
        }

        if (filtered_ppm > MQ2_MAX_PPM) {
            filtered_ppm = MQ2_MAX_PPM;
        }

        current_ppm = filtered_ppm;
    }

    if (ppm_out != NULL) {
        *ppm_out = current_ppm;
    }

    if (voltage_out) {
        *voltage_out = voltage;
    }
    
    last_sample_time = now;

    return MQ2_OK;
}

mq2_reading mq2_get_payload()
{
    mq2_reading result;
    float ppm = 0.0f;
    float voltage = 0.0f;
    result.status = mq2_sample(&ppm, &voltage);
    result.ppm = ppm;
    result.voltage = voltage;

    if (result.status != MQ2_OK) {
        switch (result.status) {
            case MQ2_ERR_INVAL:
                printf("Invalid configuration values.\n");
                break;
            case MQ2_ERR_BUSY:
                break;
            case MQ2_ERR_HW:
                printf("Hardware fault detected.\n");
                break;
            case MQ2_ERR_NO_INIT:
                printf("MQ2 driver not initialized.\n");
                break;
            default:
                printf("Error: %d\n", result.status);
                break;
        }
    } else {
        printf("MQ2 | ~%.2f ppm | %.2f V\n", result.ppm, result.voltage);
    }
    
    return result;
}

void mq2_start()
{
    printf("\n=== MQ-2 Sensor ===\n");
    printf("ADC pin=GP%d | Vref=%.2fV | Vcc=%.2fV | RL=%.0fΩ | R0=%.0fΩ\n", MQ2_ADC_GPIO, V_REF, VCC_ADC_VOLTAGE, RL_OHM, R0_CLEAN_AIR_OHM);

    mq2_config cfg = {
        .adc_channel     = 0,
        .warmup_ms       = 20000,
        .min_interval_ms = 9000
    };

    if (mq2_init(&cfg) != MQ2_OK) {
        printf("Failed to initialize MQ2 driver.\n");
    } else {
        printf("MQ2 driver initialized successfully.\n");
    }

    mq2_warmup();

    uint32_t elapsed_seconds = cfg.warmup_ms / 1000;
    while (!mq2_ready()) {
        printf("Warming up heater, please wait %useconds...\n", elapsed_seconds);
        sleep_ms(1000);

        if (elapsed_seconds > 0) {
            elapsed_seconds--;
        }
    }

    printf("Warmup complete. MQ2 Sensor ready.\n");
}