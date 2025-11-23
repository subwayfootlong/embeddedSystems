#include "mq7_driver.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include <math.h>
#include <stdio.h>


// --- Initialization Tracking ---
static bool _mq7_initialized = false;

void mq7_init_adc(void) {
    adc_init();
    adc_gpio_init(MQ7_ADC_GPIO);
    adc_select_input(1); 
    _mq7_initialized = true;

    printf("\n=== MQ-7 Sensor ===\n");
    printf("ADC pin=GP%d | Vref=%.2fV | Vcc=%.2fV | RL=%.0fΩ | R0=%.0fΩ\n", MQ7_ADC_GPIO, ADC_FULL_SCALE_VOLTS, SENSOR_SUPPLY_VOLTS, MQ7_RL_OHMS, MQ7_R0_OHMS);
}

float mq7_read_vadc_volts(uint16_t samples) {
    if (samples == 0) samples = 1;
    uint32_t acc = 0;
    for (uint16_t i = 0; i < samples; ++i) {
        acc += adc_read();
        sleep_us(100);
    }
    float avg = (float)acc / (float)samples;
    return (avg / ADC_MAX_COUNT) * ADC_FULL_SCALE_VOLTS; // 5V
}

float mq7_backscale_sensor_volts(float vadc) {
    return vadc;
}

// Vs = Vcc * (RL / (RL + RS))  => RS = RL * (Vcc - Vs) / Vs
float mq7_compute_rs_ohms(float vsensor) {
    // Safety checks to prevent divide by zero or negative resistance
    if (vsensor < 0.001f) vsensor = 0.001f;
    if (vsensor > SENSOR_SUPPLY_VOLTS - 0.001f)
        vsensor = SENSOR_SUPPLY_VOLTS - 0.001f;
        
    return MQ7_RL_OHMS * (SENSOR_SUPPLY_VOLTS - vsensor) / vsensor;
}

// Rough ppm from Rs/R0 using a log-log fit
float mq7_estimate_ppm(float rs_ohms) {
    if (rs_ohms < 1.0f) rs_ohms = 1.0f;
    float ratio = rs_ohms / MQ7_R0_OHMS;
    
    // Log-log fit: log10(ppm) = A * log10(Rs/R0) + B
    float log10ppm = MQ7_CURVE_A * log10f(ratio) + MQ7_CURVE_B;
    
    // Clamp to prevent extreme values
    if (log10ppm < -6.0f) log10ppm = -6.0f;
    if (log10ppm >  4.0f) log10ppm =  4.0f;
    
    return powf(10.0f, log10ppm);
}

//Reading
int mq7_sample(float *ppm_out, float *voltage_out) {
    
    //Check for basic errors
    if (!_mq7_initialized) {
        return MQ7_STATUS_ENOINIT;
    }
    if (MQ7_R0_OHMS == 0.0f) {
        return MQ7_STATUS_EINVAL;
    }
    
    // Perform the reading and calculations
    float vadc   = mq7_read_vadc_volts(16);          
    float vsens  = mq7_backscale_sensor_volts(vadc); 
    
    // Check for hardware errors after reading
    // EHW check: Now checks against the 5V rail
    const float rail_tolerance = 0.05f; // 50mV tolerance
    if (vadc < rail_tolerance || vadc > ADC_FULL_SCALE_VOLTS - rail_tolerance) {
        return MQ7_STATUS_EHW;
    }
    
    float rs     = mq7_compute_rs_ohms(vsens);
    float ppm    = mq7_estimate_ppm(rs);
    {
        static float filtered_ppm = 0.0f;
        const float max_ppm_voltage = ppm;

        filtered_ppm = EMA_ALPHA * ppm + (1.0f - EMA_ALPHA) * filtered_ppm;

        if (filtered_ppm > max_ppm_voltage) {
            filtered_ppm = max_ppm_voltage;
        }

        if (filtered_ppm > MQ7_MAX_PPM) {
            filtered_ppm = MQ7_MAX_PPM;
        }

        ppm = filtered_ppm;
    }
    *ppm_out = ppm;
    *voltage_out = vadc;
           
    return MQ7_STATUS_OK;
}

mq7_reading mq7_get_payload()
{
    mq7_reading result;
    float ppm = 0.0f;
    float voltage = 0.0f;
    result.status = mq7_sample(&ppm, &voltage);
    result.ppm = ppm;
    result.voltage = voltage;
    
    if (result.status != MQ7_STATUS_OK) {
        printf("\n--- MQ7 Status:\n");
        switch (result.status) {
            case MQ7_STATUS_OK:
                printf("OK: Measurement valid.");
                break;
            case MQ7_STATUS_EINVAL:
                printf("EINVAL: Bad configuration (R0=0 or other invalid constant).\n");
                break;
            case MQ7_STATUS_EBUSY:
                printf("EBUSY: Sampling attempted outside ideal LOW phase.\n");
                break;
            case MQ7_STATUS_EHW:
                printf("EHW: ADC stuck at rail (open/short detected).\n");
                break;
            case MQ7_STATUS_ENOINIT:
                printf("ENOINIT: API called before mq7_init.\n");
                break;
            default:
                printf("\nRemediation: Verify power driver circuit, recalibrate R0, check wiring.\n");
                break;
        }
    } else {
        printf("MQ7 | ~%.1f ppm | %.3f V\n", result.ppm, result.voltage);
    }
    
    return result;
}