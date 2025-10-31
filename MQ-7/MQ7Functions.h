#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MQ7_STATUS_OK = 0,          // Measurement valid.
    MQ7_STATUS_EINVAL,          // Bad configuration (invalid R0, etc.).
    MQ7_STATUS_EBUSY,           // Sampling attempted outside LOW phase (Conceptual in 3.3V mode).
    MQ7_STATUS_EHW,             // ADC stuck at rail, open/short detected.
    MQ7_STATUS_ENOINIT          // API called before mq7_init.
} mq7_status_t;

// -------- Hardware config (5 V, no divider) --------
#define MQ7_ADC_GPIO          27       // CORRECTED: GP27 = ADC1
#define ADC_FULL_SCALE_VOLTS  5.0f
#define ADC_MAX_COUNT         4095.0f

// No divider (A0 -> GP27 directly)
#define DIVIDER_RTOP_OHMS     0.0f
#define DIVIDER_RBOT_OHMS     1.0f

// Module powered from 5V
#define SENSOR_SUPPLY_VOLTS   5.0f

// Flying-Fish modules commonly RL ~10k (change if your board differs)
#define MQ7_RL_OHMS           10000.0f

// Calibrate: set R0 = Rs in clean air after warm-up (you measure this)
// !!! CRITICAL: UPDATE THIS VALUE after your 48-hour burn-in.
#define MQ7_R0_OHMS           102600.0f

// Curve (placeholder): log10(ppm) = A * log10(Rs/R0) + B
// These placeholders will lead to inaccurate ppm values.
#define MQ7_CURVE_A          (-1.4754f)  
#define MQ7_CURVE_B           ( 2.0f) 

void   mq7_init_adc(void);
float  mq7_read_vadc_volts(uint16_t samples);     // 5 V at ADC
float  mq7_backscale_sensor_volts(float vadc);    // = vadc (no divider)
float  mq7_compute_rs_ohms(float vsensor);        // from Vs, RL, Vcc
float  mq7_estimate_ppm(float rs_ohms);           // uses Rs/R0 + curve

void mq7_print_status_message(mq7_status_t status);
mq7_status_t mq7_read_and_print_stats(void);

#ifdef __cplusplus
}
#endif
