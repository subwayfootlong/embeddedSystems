#ifndef MQ135_DRIVER_H
#define MQ135_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

// ========= Error Definitions =========
typedef enum {
    MQ135_OK      = 0,  // Measurement valid.
    MQ135_EINVAL  = 1,  // Invalid configuration (e.g., missing R0 or NULL pointer).
    MQ135_EBUSY   = 2,  // Sample attempted before interval elapsed.
    MQ135_EHW     = 3,  // Hardware error: ADC rails or open/short condition.
    MQ135_ENOINIT = 4   // API used before init.
} mq135_status_t;

// ========= Hardware Configuration (Remains the same) =========
#define MQ135_ADC_GPIO              7
#define ADC_FULL_SCALE_MQ135_VOLTS  5.0f
#define ADC_MAX_COUNT               4095.0f

#define DIVIDER_RTOP_OHMS           0.0f
#define DIVIDER_RBOT_OHMS           1.0f

#define SENSOR_SUPPLY_VOLTS         5.0f
#define MQ135_RL_OHMS               10000.0f
#define MQ135_R0_OHMS               140000.0f

#define MQ135_NH3_A                 100.0f
#define MQ135_NH3_B                 (-2.4098f)

#define MQ135_MIN_PPM               0.1f
#define MQ135_MAX_PPM               10000.0f

#define MQ135_WARMUP_MS (3 * 60 * 1000) 
#define MQ135_MIN_INTERVAL_MS 100 

// ========= Data Structure (Remains the same) =========
typedef struct {
    int status;
    float vadc;
    float vs;
    float rs;
    float ratio;
    float ppm;
} mq135_reading_t;

// ========= Public API (New Function Added) =========

mq135_status_t mq135_setup(void);

bool mq135_ready(void);

mq135_status_t mq135_read(mq135_reading_t *p_reading);

const char* mq135_strerror(mq135_status_t status);

void mq135_print(const mq135_reading_t *p_reading);

void mq135_print_config(void);

mq135_reading_t mq135_get_payload(void);

/**
 * @brief Runs the main sensor loop, handling setup, and continuous reading/error reporting.
 */
void mq135_start(void);

#endif // MQ135_DRIVER_H