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

// ========= Hardware Configuration =========
#define MQ135_ADC_GPIO              27       // GP27/ADC1 (Your ADC pin)
#define ADC_FULL_SCALE_VOLTS        3.3f     // Your MCU's max ADC input voltage
#define ADC_MAX_COUNT               4095.0f  // 12-bit ADC count

// Voltage divider - SET FOR NO DIVIDER (Vs = Vadc):
#define DIVIDER_RTOP_OHMS           0.0f
#define DIVIDER_RBOT_OHMS           1.0f

// Sensor parameters - 5V POWERED SETUP
#define SENSOR_SUPPLY_VOLTS         5.0f     // **5V** external power supply (VCC to sensor module)
#define MQ135_RL_OHMS               10000.0f // Load resistor on module (usually 10k)
#define MQ135_R0_OHMS               76630.0f // Calibration value (update after warm-up)

// Gas curve coefficients: PPM = A * (Rs/R0)^B
// UPDATED BASED ON MANUAL DATA: (Rs/R0=2.6 at 10ppm; Rs/R0=1 at 100ppm)
#define MQ135_NH3_A                 100.0f
#define MQ135_NH3_B                 (-2.4098f)

// Sanity limits
#define MQ135_MIN_PPM               0.1f
#define MQ135_MAX_PPM               10000.0f

// ========= Data Structure =========
typedef struct {
    float vadc;       // Voltage at ADC pin (0-3.3V)
    float vs;         // Voltage at sensor node (VRL)
    float rs;         // Sensor resistance (Ohms)
    float ratio;      // Rs/R0
    float nh3_ppm;    // NH3 concentration estimate
} mq135_reading_t;

// ========= Public API (Function Prototypes Updated) =========

/**
 * @brief Initialize MQ-135 sensor. Returns status.
 */
mq135_status_t mq135_setup(void);

/**
 * @brief Check if the sensor has completed the warm-up period.
 * @return True if ready, false otherwise.
 */
bool mq135_ready(void);

/**
 * @brief Read all sensor values and gas estimates. Returns status.
 * @param reading Pointer to structure to fill with results
 */
mq135_status_t mq135_read(mq135_reading_t *reading);

/**
 * @brief Utility to convert status code to a human-readable string.
 */
const char* mq135_strerror(mq135_status_t status);

/**
 * @brief Print formatted sensor reading to console
 */
void mq135_print(const mq135_reading_t *reading);

/**
 * @brief Print sensor configuration banner
 */
void mq135_print_config(void);

#ifdef __cplusplus
}
#endif