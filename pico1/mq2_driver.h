#ifndef MQ2_FUNCTIONS_H
#define MQ2_FUNCTIONS_H

#include <stdint.h>
#include <stdbool.h>

#define MQ2_ADC_GPIO (26)
#define V_REF (3.3f)
#define VCC_ADC_VOLTAGE (5.0f)
#define kAdcMax (4095.0f)
#define RL_OHM (20000.0f) 
#define R0_CLEAN_AIR_OHM (535555.6f)
#define LPG_SLOPE (-0.4500f)
#define LPG_INTERCEPT (1.1710f)
#define EMA_ALPHA (0.1f)
#define MQ2_MAX_PPM (10000.0f)

/**
 * @brief Error codes for MQ2 sensor driver
 */
typedef enum {
    MQ2_OK          = 0,
    MQ2_ERR_INVAL   = -1,
    MQ2_ERR_BUSY    = -2,
    MQ2_ERR_HW      = -3,
    MQ2_ERR_NO_INIT = -4
} mq2_status;

/**
 * @brief MQ2 sensor configuration
 */
typedef struct {
    uint8_t     adc_channel;        // ADC channel number (e.g., 0 for GPIO26)
    uint32_t    warmup_ms;          // Warm-up time in milliseconds
    uint32_t    min_interval_ms;    // Minimum time between samples in milliseconds
} mq2_config;

typedef struct {
    int status;
    float ppm;
    float voltage;
} mq2_reading;

int mq2_init(const mq2_config *cfg);
mq2_config mq2_get_config();
int mq2_warmup();
bool mq2_ready();
int mq2_sample(float *ppm_out, float *voltage_out);
mq2_reading mq2_get_payload();
void mq2_start();

#endif