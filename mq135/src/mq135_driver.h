#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ========= Hardware Configuration =========
#define MQ135_ADC_GPIO            27       // GP27/ADC1
#define ADC_FULL_SCALE_VOLTS      3.3f
#define ADC_MAX_COUNT             4095.0f

// Voltage divider - SET THESE BASED ON YOUR SETUP:
// NO DIVIDER (direct connection, risky): RTOP=0, RBOT=1
// WITH DIVIDER (20k/10k recommended): RTOP=20000, RBOT=10000
#define DIVIDER_RTOP_OHMS         0.0f     // 0 = no divider (change to 20000.0f if using divider)
#define DIVIDER_RBOT_OHMS         1.0f     // 1 = no divider (change to 10000.0f if using divider)

// Sensor parameters - UPDATED FOR 5V
#define SENSOR_SUPPLY_VOLTS       5.0f     // 5V external power supply
#define MQ135_RL_OHMS             10000.0f // Load resistor on module (usually 10k)
#define MQ135_R0_OHMS             76630.0f // Calibration value (update after warm-up)

// Gas curve coefficients: PPM = a * (Rs/R0)^b
// From GitHub library: https://github.com/miguel5612/MQSensorsLib
#define MQ135_NH3_A               102.2f    // Scaling factor for NH4/NH3
#define MQ135_NH3_B               (-2.473f) // Exponential factor for NH4/NH3
#define MQ135_CO2_A               110.47f   // Scaling factor for CO2
#define MQ135_CO2_B               (-2.862f) // Exponential factor for CO2

// Sanity limits
#define MQ135_MIN_PPM             0.1f
#define MQ135_MAX_PPM             10000.0f

// ========= Data Structure =========
typedef struct {
    float vadc;      // Voltage at ADC pin (0-3.3V)
    float vs;        // Voltage at sensor node (after divider compensation)
    float rs;        // Sensor resistance (Ohms)
    float ratio;     // Rs/R0
    float nh3_ppm;   // NH3 concentration estimate
} mq135_reading_t;

// ========= Public API =========

/**
 * @brief Initialize MQ-135 sensor (must call before reading)
 */
void mq135_setup(void);

/**
 * @brief Read all sensor values and gas estimates
 * @param reading Pointer to structure to fill with results
 */
void mq135_read(mq135_reading_t *reading);

/**
 * @brief Print formatted sensor reading to console
 * @param reading Pointer to reading structure
 */
void mq135_print(const mq135_reading_t *reading);

/**
 * @brief Print sensor configuration banner
 */
void mq135_print_config(void);

#ifdef __cplusplus
}
#endif