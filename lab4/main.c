#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "functions.h"

// Pin definitions
#define PWM_PIN 0     // GP0 for PWM
#define ADC_PIN 26    // GP26 for ADC

// Timer interval
#define TIMER_INTERVAL_MS 25

// PWM setup
void setup_pwm(uint gpio, float freq, float duty_cycle) {
    // Set GPIO as PWM function
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    // Get PWM slice number
    uint slice_num = pwm_gpio_to_slice_num(gpio);

    // Pico system clock runs at 125 MHz by default
    float clock_freq = 125000000.0f;  // Pico default system clock
    // Divider calculation: ensures PWM resolution of 16 bits (wrap=65535)
    float divider = clock_freq / (freq * 65536.0f);

    // Clamp divider value between 1 and 255
    if (divider < 1.0f) divider = 1.0f;
    if (divider > 255.0f) divider = 255.0f;

    // Set PWM clock divider
    pwm_set_clkdiv(slice_num, divider);
    // Set 16-bit counter wrap
    pwm_set_wrap(slice_num, 65535);
    // Set duty cycle
    pwm_set_gpio_level(gpio, (uint16_t)(duty_cycle * 65535));
    // Enable PWM output
    pwm_set_enabled(slice_num, true);
}

bool timer_wrapper(struct repeating_timer *rt) {
    uint16_t adc_val = adc_read();
    uint32_t timestamp_us = to_us_since_boot(get_absolute_time());
    
    return timer_callback(adc_val, timestamp_us);
}

int main() {
    stdio_init_all();

    // Enable ADC hardware
    adc_init();
    // Configure for ADC input
    adc_gpio_init(ADC_PIN);   // Enable ADC input
    // Select ADC channel 0     
    adc_select_input(0);      // Select ADC0 (GP26)

    // Setup PWM
    setup_pwm(PWM_PIN, 20.0f, 0.5f);

    // Create repeating timer 
    struct repeating_timer timer;
    add_repeating_timer_ms(-TIMER_INTERVAL_MS, timer_wrapper, NULL, &timer);

    printf("\nLoop Start\n");

    while (true) {
        tight_loop_contents();   // Idle loop, everything happens in timer
    }

    return 0;
}
