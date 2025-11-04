#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ========= Error Definitions =========
typedef enum {
    MQ135_OK      = 0,  // Measurement valid.
    MQ135_EINVAL  = 1,  // Invalid configuration (e.g., missing R0 or NULL pointer).
    MQ135_EBUSY   = 2,  // Sample attempted before interval elapsed.
    MQ135_EHW     = 3,  // Hardware error: ADC rails or open/short condition.
    MQ135_ENOINIT = 4   // API used before init.
} mq135_status_t;

// ========= Hardware Configuration (Remains the same) =========
#define MQ135_ADC_GPIO              27
#define ADC_FULL_SCALE_VOLTS        3.3f
#define ADC_MAX_COUNT               4095.0f

#define DIVIDER_RTOP_OHMS           0.0f
#define DIVIDER_RBOT_OHMS           1.0f

#define SENSOR_SUPPLY_VOLTS         5.0f
#define MQ135_RL_OHMS               10000.0f
#define MQ135_R0_OHMS               250000.0f

#define MQ135_NH3_A                 100.0f
#define MQ135_NH3_B                 (-2.4098f)

#define MQ135_MIN_PPM               0.1f
#define MQ135_MAX_PPM               10000.0f

// ========= Data Structure (Remains the same) =========
typedef struct {
    float vadc;
    float vs;
    float rs;
    float ratio;
    float nh3_ppm;
} mq135_reading_t;

// ========= Public API (New Function Added) =========

mq135_status_t mq135_setup(void);

bool mq135_ready(void);

mq135_status_t mq135_read(mq135_reading_t *reading);

const char* mq135_strerror(mq135_status_t status);

void mq135_print(const mq135_reading_t *reading);

void mq135_print_config(void);

/**
 * @brief Runs the main sensor loop, handling setup, and continuous reading/error reporting.
 */
void mq135_run_loop(void);

#ifdef __cplusplus
}
#endif