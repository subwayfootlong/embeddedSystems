#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdint.h>
#include <stdbool.h>

// Timer callback: takes ADC value and timestamp in microseconds
bool timer_callback(uint16_t adc_val, uint32_t timestamp_us);

#endif // FUNCTIONS_H
