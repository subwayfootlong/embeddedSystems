#ifndef MQ2_DRIVER_H
#define MQ2_DRIVER_H

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
} mq2_status_t;

/**
 * @brief MQ2 sensor configuration
 */
typedef struct {
    uint8_t     adc_channel;        // ADC channel number (e.g., 0 for GPIO26)
    uint32_t    warmup_ms;          // Warm-up time in milliseconds
} mq2_config_t;

typedef struct {
    int status;
    float ppm;
    float voltage;
} mq2_reading_t;

// Initialize ADC and MQ2 sensor configuration
int mq2_init(const mq2_config_t *p_cfg);

// Initialize the warm-up process
int mq2_warmup();

// Check if the sensor is ready after warm-up
bool mq2_ready();

// Sample the sensor and get ppm and voltage readings
int mq2_sample(float *p_ppm_out, float *p_voltage_out);

// Get the current sensor reading as a structured payload
mq2_reading_t mq2_get_payload();

// Encapsulates mq2_init, mq2_warmup, and mq2_ready into a single setup function
void mq2_start();

#endif  // MQ2_DRIVER_H