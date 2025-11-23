#include "mq135_driver.h"
#include "hardware/adc.h"
#include "hardware/timer.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <math.h>

// State Variables and Constants
static bool g_mq135_is_initialized = false;
static absolute_time_t g_mq135_init_time;
static absolute_time_t g_mq135_last_sample_time;

// Private Helper Functions
static inline uint32_t gpio_to_adc_ch(uint32_t gpio)
{
    return (gpio >= 26 && gpio <= 28) ? (gpio - 26) : 0;
}

static uint16_t read_raw_adc(void)
{ 
    adc_select_input(gpio_to_adc_ch(MQ135_ADC_GPIO));
    sleep_us(50);
    return adc_read();
}

static float read_vadc_averaged(void)
{
    uint32_t acc = 0;
    for (int i = 0; i < 16; ++i)
    {
        acc += read_raw_adc();
        sleep_us(80);
    }
    float avg = (float)acc / 16.0f;
    
    return (avg / ADC_MAX_COUNT) * ADC_FULL_SCALE_MQ135_VOLTS;
}

static float backscale_vs(float vadc)
{
    const float rtop = DIVIDER_RTOP_OHMS;
    const float rbot = DIVIDER_RBOT_OHMS;
    if (0.0f == rtop && 1.0f == rbot)
    {
        return vadc;
    }

    return vadc * ((rtop + rbot) / rbot);
}

static float compute_rs(float vs)
{
    if (vs < 0.001f) vs = 0.001f;
    if (vs > SENSOR_SUPPLY_VOLTS - 0.001f) vs = SENSOR_SUPPLY_VOLTS - 0.001f;
    return MQ135_RL_OHMS * (SENSOR_SUPPLY_VOLTS - vs) / vs;
}

static float compute_ratio(float rs)
{
    float r0 = (MQ135_R0_OHMS <= 1.0f) ? 1.0f : MQ135_R0_OHMS;
    return rs / r0;
}

static float clamp_ppm(float ppm)
{
    if (ppm < MQ135_MIN_PPM) ppm = MQ135_MIN_PPM;
    if (ppm > MQ135_MAX_PPM) ppm = MQ135_MAX_PPM;
    return ppm;
}

static float calculate_nh3_ppm(float ratio)
{
    if (ratio <= 0.0f) ratio = 1e-6f;
    float ppm = MQ135_NH3_A * powf(ratio, MQ135_NH3_B);
    return clamp_ppm(ppm);
}

// Public API Implementation (Setup, Read, Print)
const char* mq135_strerror(mq135_status_t status)
{
    switch (status)
    {
        case MQ135_OK:      return "OK: Measurement valid.";
        case MQ135_EINVAL:  return "EINVAL: Invalid config (Missing R0 or NULL pointer).";
        case MQ135_EBUSY:   return "EBUSY: Sample attempted before minimum interval elapsed.";
        case MQ135_EHW:     return "EHW: Hardware error: ADC voltage stuck at rails (check wiring).";
        case MQ135_ENOINIT: return "ENOINIT: API used before sensor setup was called.";
        default:            return "UNKNOWN ERROR";
    }
}

mq135_status_t mq135_setup(void) {
    if (MQ135_R0_OHMS <= 1.0f)
    {
        return MQ135_EINVAL;
    }
    adc_init();
    adc_gpio_init(MQ135_ADC_GPIO);
    g_mq135_init_time = get_absolute_time();
    g_mq135_last_sample_time = g_mq135_init_time;
    g_mq135_is_initialized = true;
    return MQ135_OK;
}

bool mq135_ready(void) {
    if (!g_mq135_is_initialized) return false;
    return absolute_time_diff_us(g_mq135_init_time, get_absolute_time()) >= (MQ135_WARMUP_MS * 1000);
}


mq135_status_t mq135_read(mq135_reading_t *p_reading) {
    if (!p_reading) return MQ135_EINVAL;
    if (!g_mq135_is_initialized) return MQ135_ENOINIT;

    if (absolute_time_diff_us(g_mq135_last_sample_time, get_absolute_time()) < (MQ135_MIN_INTERVAL_MS * 1000)) {
        return MQ135_EBUSY;
    }
    p_reading->vadc = read_vadc_averaged();
    p_reading->vs = backscale_vs(p_reading->vadc); 
    
    if (p_reading->vadc <= ADC_RAIL_THRESHOLD || 
        p_reading->vadc >= (ADC_FULL_SCALE_MQ135_VOLTS - ADC_RAIL_THRESHOLD)) 
    {
        return MQ135_EHW;
    }

    p_reading->rs = compute_rs(p_reading->vs);
    p_reading->ratio = compute_ratio(p_reading->rs);
    p_reading->ppm = calculate_nh3_ppm(p_reading->ratio);
    
    g_mq135_last_sample_time = get_absolute_time();
    
    return MQ135_OK;
}

void mq135_print(const mq135_reading_t *p_reading) {
    if (!p_reading) return;
    printf("MQ135 | ~%.1f ppm | %.3f V\n",
           p_reading->ppm, p_reading->vadc);
}

void mq135_print_config(void) {
    printf("\n=== MQ-135 Sensor (NH3 Only) ===\n");
    printf("ADC pin=GP%d | Vref=%.2fV | Vcc=%.2fV | RL=%.0fΩ | R0=%.0fΩ\n",
           MQ135_ADC_GPIO, ADC_FULL_SCALE_MQ135_VOLTS, SENSOR_SUPPLY_VOLTS, 
           MQ135_RL_OHMS, MQ135_R0_OHMS);
    printf("⚠️ WARNING: No voltage divider! Direct connection. Max Vout MUST stay < %.2fV!\n", ADC_FULL_SCALE_MQ135_VOLTS);
    printf("NH3 Formula: PPM = %.2f * (Rs/R0)^%.4f\n", MQ135_NH3_A, MQ135_NH3_B);
    printf("NOTE: Sensor needs 3-5 minute warm-up.\n\n");
}

void mq135_start(void) {
    mq135_status_t status;
    mq135_print_config();
    
    // Attempt setup and handle critical errors immediately
    status = mq135_setup();
    if (status != MQ135_OK)
    {
        printf("🚨 CRITICAL SETUP ERROR: %s\n", mq135_strerror(status));
    }
    
    
}

mq135_reading_t mq135_get_payload(void)
{
    mq135_reading_t reading;

    reading.status = mq135_read(&reading);
    
    if (reading.status == MQ135_OK) {
        mq135_print(&reading);
    } else {
        // Failure: Print the specific error message
        printf("🛑 READ ERROR (%d): %s\n", reading.status, mq135_strerror(reading.status));
    }

    return reading;
}