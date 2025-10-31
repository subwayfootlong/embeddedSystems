#include "MQ7Functions.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include <math.h>
#include <stdio.h>


// --- Initialization Tracking ---
static bool _mq7_initialized = false;
// ---- ADC init/read ----
void mq7_init_adc(void) {
    adc_init();
    adc_gpio_init(MQ7_ADC_GPIO);
    adc_select_input(1); // GP27 = ADC1
    _mq7_initialized = true;
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

// Rough ppm from Rs/R0 using a log-log fit (placeholder constants)
float mq7_estimate_ppm(float rs_ohms) {
    if (rs_ohms < 1.0f) rs_ohms = 1.0f;
    float ratio = rs_ohms / MQ7_R0_OHMS;
    
    // Log-log fit: log10(ppm) = A * log10(Rs/R0) + B
    float log10ppm = MQ7_CURVE_A * log10f(ratio) + MQ7_CURVE_B;
    
    // Clamp to prevent extreme values (optional, clamps to 1,000,000 ppm)
    if (log10ppm < -6.0f) log10ppm = -6.0f;
    if (log10ppm >  6.0f) log10ppm =  6.0f;
    
    return powf(10.0f, log10ppm);
}

void mq7_print_status_message(mq7_status_t status) {
    printf("\n--- Status: ");
    switch (status) {
        case MQ7_STATUS_OK:
            printf("OK: Measurement valid.");
            break;
        case MQ7_STATUS_EINVAL:
            printf("EINVAL: Bad configuration (R0=0 or other invalid constant).");
            break;
        case MQ7_STATUS_EBUSY:
            printf("EBUSY: Sampling attempted outside ideal LOW phase.");
            break;
        case MQ7_STATUS_EHW:
            printf("EHW: ADC stuck at rail (open/short detected).");
            break;
        case MQ7_STATUS_ENOINIT:
            printf("ENOINIT: API called before mq7_init.");
            break;
    }
    
    // Print remediation advice if an error occurred
    if (status != MQ7_STATUS_OK) {
        printf("\n   Remediation: Verify power driver circuit, recalibrate R0, check wiring.\n");
    } else {
        printf("\n");
    }
}

// --- Reading and Printing Function ---
mq7_status_t mq7_read_and_print_stats(void) {
    
    // 1. Check for basic errors
    if (!_mq7_initialized) {
        return MQ7_STATUS_ENOINIT;
    }
    if (MQ7_R0_OHMS == 0.0f) {
        return MQ7_STATUS_EINVAL;
    }
    
    // 2. Perform the reading and calculations
    float vadc   = mq7_read_vadc_volts(16);          // V_ADC (0..5V)
    float vsens  = mq7_backscale_sensor_volts(vadc); // V_S (Sensor Voltage, 0..5V)
    
    // 3. Check for hardware errors after reading
    // EHW check: Now checks against the 5V rail
    const float rail_tolerance = 0.05f; // 50mV tolerance
    if (vadc < rail_tolerance || vadc > ADC_FULL_SCALE_VOLTS - rail_tolerance) {
        return MQ7_STATUS_EHW;
    }
    
    float rs     = mq7_compute_rs_ohms(vsens);       
    float ppm    = mq7_estimate_ppm(rs);             

    // 4. Print stats and return OK status
    printf("Vadc=%.3f V | Vs=%.3f V | Rs=%8.3f Ω | Rs/R0=%.2f | ppm~%.1f\n",
           vadc, vsens, rs, rs / MQ7_R0_OHMS, ppm);
           
    return MQ7_STATUS_OK;
}