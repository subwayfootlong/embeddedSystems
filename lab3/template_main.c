#include <stdio.h>
#include "pico/stdlib.h"
#include "functions.h"

// GPIO pin number for the button (changed to GP21 as required)
#define BTN_PIN 21

// Global stopwatch instance
Stopwatch sw;

// Alarm callback: called once after a delay (for debounce handling)
int64_t alarm_callback(alarm_id_t id, void *user_data) {
    bool button_pressed = gpio_get(BTN_PIN);    // Read button state
    stopwatch_alarm_update(&sw, button_pressed);// Update stopwatch with debounced button
    return 0; // don't repeat
}

// Timer callback: called repeatedly at a fixed interval (1 second here)
bool repeating_timer_callback(struct repeating_timer *t) {
    stopwatch_tick(&sw);    // Increment stopwatch seconds
    return true;            // Keep repeating
}

int main() {
    stdio_init_all();
    gpio_set_dir(BTN_PIN, GPIO_IN);
    gpio_set_pulls(BTN_PIN, true, false);

    stopwatch_init(&sw);

    struct repeating_timer timer;
    add_repeating_timer_ms(1000, repeating_timer_callback, NULL, &timer);

    printf("Stopwatch initialized. Press and hold button on GP21 to start timer.\n");

    while (true) {
        bool button_pressed = gpio_get(BTN_PIN);

        // Only start debounce when button transitions from not pressed to pressed and in idle state
        if (!button_pressed && !sw.debounce_start && sw.system_state == STATE_IDLE) {
            // ^^^^ should be !button_pressed for active-low button
            sw.debounce_start = true;
            add_alarm_in_ms(50, alarm_callback, NULL, false);
        }

        stopwatch_update_state(&sw, button_pressed);
        sleep_ms(10);
    }
}