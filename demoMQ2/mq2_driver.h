#ifndef MQ2_FUNCTIONS_H
#define MQ2_FUNCTIONS_H

#include <stdint.h>
#include <stdbool.h>

#define VCC_ADC_VOLTAGE (5.0f)
#define RL_OHM (10000.0f) 
#define R0_CLEAN_AIR_OHM (168571.0f) // (5.0/0.28 - 1) * 10000
#define LPG_SLOPE (-0.4500f)
#define LPG_INTERCEPT (0.4500f)

/**
 * @brief Error codes for MQ2 sensor driver
 */
typedef enum {
    MQ2_OK          = 0,
    MQ2_ERR_INVAL   = -1,
    MQ2_ERR_BUSY    = -2,
    MQ2_ERR_HW      = -3,
    MQ2_ERR_NO_INIT = -4
} Mq2Status;

/**
 * @brief MQ2 sensor configuration
 */
typedef struct {
    uint8_t     adc_channel;        // ADC channel number (e.g., 0 for GPIO26)
    uint32_t    warmup_ms;          // Warm-up time in milliseconds
    uint32_t    min_interval_ms;    // Minimum time between samples in milliseconds
} Mq2Config;

int mq2_init(const Mq2Config *cfg);
int mq2_warmup();
bool mq2_ready();
int mq2_sample(float *ppm_out, float *voltage_out);
int mq2_start();
void mq2_loop();

#endif /* MQ2_FUNCTIONS_H */