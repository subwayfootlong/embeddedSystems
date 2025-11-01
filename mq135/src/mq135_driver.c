#include "mq135_driver.h"
#include "hardware/adc.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <math.h>

// ========= Private Helper Functions =========

static inline uint32_t gpio_to_adc_ch(uint32_t gpio) {
    return (gpio >= 26 && gpio <= 28) ? (gpio - 26) : 0;
}

static uint16_t read_raw_adc(void) {
    adc_select_input(gpio_to_adc_ch(MQ135_ADC_GPIO));
    sleep_us(50);
    return adc_read();
}

static float read_vadc_averaged(void) {
    uint32_t acc = 0;
    for (int i = 0; i < 16; ++i) {
        acc += read_raw_adc();
        sleep_us(80);
    }
    float avg = (float)acc / 16.0f;
    return (avg / ADC_MAX_COUNT) * ADC_FULL_SCALE_VOLTS;
}

static float backscale_vs(float vadc) {
    const float rtop = DIVIDER_RTOP_OHMS;
    const float rbot = DIVIDER_RBOT_OHMS;
    if (rtop == 0.0f && rbot == 1.0f) return vadc;
    return vadc * ((rtop + rbot) / rbot);
}

static float compute_rs(float vs) {
    if (vs < 0.001f) vs = 0.001f;
    if (vs > SENSOR_SUPPLY_VOLTS - 0.001f) vs = SENSOR_SUPPLY_VOLTS - 0.001f;
    return MQ135_RL_OHMS * (SENSOR_SUPPLY_VOLTS - vs) / vs;
}

static float compute_ratio(float rs) {
    float r0 = (MQ135_R0_OHMS <= 1.0f) ? 1.0f : MQ135_R0_OHMS;
    return rs / r0;
}

static float clamp_ppm(float ppm) {
    if (ppm < MQ135_MIN_PPM) ppm = MQ135_MIN_PPM;
    if (ppm > MQ135_MAX_PPM) ppm = MQ135_MAX_PPM;
    return ppm;
}

static float calculate_nh3_ppm(float ratio) {
    if (ratio <= 0.0f) ratio = 1e-6f;
    // Formula: PPM = a * (Rs/R0)^b
    float ppm = MQ135_NH3_A * powf(ratio, MQ135_NH3_B);
    return clamp_ppm(ppm);
}

static float calculate_co2_ppm(float ratio) {
    if (ratio <= 0.0f) ratio = 1e-6f;
    // Formula: PPM = a * (Rs/R0)^b
    float ppm = MQ135_CO2_A * powf(ratio, MQ135_CO2_B);
    return clamp_ppm(ppm);
}

// ========= Public API Implementation =========

void mq135_setup(void) {
    adc_init();
    adc_gpio_init(MQ135_ADC_GPIO);
}

void mq135_read(mq135_reading_t *reading) {
    if (!reading) return;
    
    reading->vadc = read_vadc_averaged();
    reading->vs = backscale_vs(reading->vadc);
    reading->rs = compute_rs(reading->vs);
    reading->ratio = compute_ratio(reading->rs);
    reading->nh3_ppm = calculate_nh3_ppm(reading->ratio);
}

void mq135_print(const mq135_reading_t *reading) {
    if (!reading) return;
    
    printf("Vadc=%.3fV | Rs=%.0fΩ | Rs/R0=%.2f | NH3=%.1f ppm\n",
           reading->vadc, reading->rs, reading->ratio, reading->nh3_ppm);
}

void mq135_print_config(void) {
    printf("\n=== MQ-135 Ammonia Sensor (5V Power) ===\n");
    printf("ADC pin=GP%d  Vref=%.2fV  Sensor Vcc=%.2fV  RL=%.0fΩ  R0=%.0fΩ\n",
           MQ135_ADC_GPIO, ADC_FULL_SCALE_VOLTS, SENSOR_SUPPLY_VOLTS, 
           MQ135_RL_OHMS, MQ135_R0_OHMS);
    
    if (DIVIDER_RTOP_OHMS > 0.0f) {
        printf("Voltage divider: %.0fkΩ / %.0fkΩ\n", 
               DIVIDER_RTOP_OHMS/1000.0f, DIVIDER_RBOT_OHMS/1000.0f);
    } else {
        printf("⚠️  WARNING: No voltage divider! Direct connection may damage ADC!\n");
    }
    
    printf("Formula: PPM = %.2f * (Rs/R0)^%.3f\n", MQ135_NH3_A, MQ135_NH3_B);
    printf("NOTE: Sensor needs 24-48h warm-up for accurate readings.\n\n");
}