#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

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
#define MQ135_R0_OHMS               250000.0f // Calibration value (update after warm-up)

// Gas curve coefficients: PPM = a * (Rs/R0)^b
// ONLY NH3 IS KEPT
#define MQ135_NH3_A                 100.0f
#define MQ135_NH3_B                 (-2.4098f)

// Sanity limits
#define MQ135_MIN_PPM               0.1f
#define MQ135_MAX_PPM               10000.0f

// ========= Data Structure (CO2 field removed) =========
typedef struct {
    float vadc;       // Voltage at ADC pin (0-3.3V)
    float vs;         // Voltage at sensor node (VRL, after divider compensation)
    float rs;         // Sensor resistance (Ohms)
    float ratio;      // Rs/R0
    float nh3_ppm;    // NH3 concentration estimate
    // float co2_ppm;  // REMOVED
} mq135_reading_t;

// ========= Public API (Function Prototypes) =========
void mq135_setup(void);
void mq135_read(mq135_reading_t *reading);
void mq135_print(const mq135_reading_t *reading);
void mq135_print_config(void);

#ifdef __cplusplus
}
#endif