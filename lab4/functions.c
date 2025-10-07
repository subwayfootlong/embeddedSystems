#include "functions.h"
#include <stdio.h>

// Callback function triggered by a timer, receiving ADC value and timestamp
bool timer_callback(uint16_t adc_val, uint32_t timestamp_us) {
    // Convert timestamp (in microseconds) into hours, minutes, seconds, and milliseconds
    // Extract milliseconds (0–999)
    uint32_t ms = (timestamp_us / 1000) % 1000;
    // Extract seconds (0–59)
    uint32_t sec = (timestamp_us / 1000000) % 60;
    // Extract minutes (0–59)
    uint32_t min = (timestamp_us / 60000000) % 60;
    // Extract hours (0–23)
    uint32_t hr = (timestamp_us / 3600000000) % 24;

    // Print timestamp and ADC value in formatted output
    // Example: "1:23:45:678 -> ADC Value: 512"
    printf("\n%ld:%ld:%ld:%ld -> ADC Value: %d", hr, min, sec, ms, adc_val);
    return true;
}