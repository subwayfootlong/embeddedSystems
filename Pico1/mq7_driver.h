#ifndef MQ7_DRIVER_H
#define MQ7_DRIVER_H

#include <stdint.h>

typedef enum {
    MQ7_STATUS_OK = 0,          // Measurement valid.
    MQ7_STATUS_EINVAL,          // Bad configuration (invalid R0, etc.).
    MQ7_STATUS_EBUSY,           // Sensor not ready (in warmup period).
    MQ7_STATUS_EHW,             // ADC stuck at rail, open/short detected.
    MQ7_STATUS_ENOINIT          // API called before mq7_init.
} mq7_status_t;

typedef struct {
    int status;
    float ppm;
    float voltage;
} mq7_reading_t;

#define MQ7_ADC_GPIO         9
#define ADC_FULL_SCALE_VOLTS 5.0f
#define ADC_MAX_COUNT        4095.0f
#define EMA_ALPHA            (0.1f)
#define MQ7_MAX_PPM          10000.0f 

// No divider (not used)
#define DIVIDER_RTOP_OHMS     0.0f
#define DIVIDER_RBOT_OHMS     1.0f

// powered with 5V
#define SENSOR_SUPPLY_VOLTS   5.0f

// load resistance
#define MQ7_RL_OHMS           10000.0f

// Calibrated baseline resistance in clean air
#define MQ7_R0_OHMS           118000.0f

// Derived values based on graph frrom datasheet
#define MQ7_CURVE_A          (-1.4754f)  
#define MQ7_CURVE_B           ( 2.0f) 

void   mq7_init_adc(void);
float  mq7_read_vadc_volts(uint16_t samples);     // 5 V at ADC
float  mq7_backscale_sensor_volts(float vadc);    // = vadc (no divider)
float  mq7_compute_rs_ohms(float vsensor);        // from Vs, RL, Vcc
float  mq7_estimate_ppm(float rs_ohms);           // uses Rs/R0 + curve
int mq7_sample(float *p_ppm_out, float *p_voltage_out);
mq7_reading_t mq7_get_payload();

#endif // MQ7_DRIVER_H