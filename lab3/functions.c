#include "functions.h"
#include <stdio.h>

// Initialize stopwatch variables
void stopwatch_init(Stopwatch *sw) {
    // Start in idle state
    sw->system_state = STATE_IDLE;
    
    // Button is not pressed initially
    sw->btn_depressed = false;
    
    // Debounce process not started
    sw->debounce_start = false;
    
    // Stopwatch starts at 0 seconds
    sw->second_count = 0;
}

// Update button state after debounce check
void stopwatch_alarm_update(Stopwatch *sw, bool button_pressed) {
    // If button is released, mark as depressed (valid press detected)
    if (!button_pressed) {
        sw->btn_depressed = true;
    } else {
        sw->btn_depressed = false;
    }
    // Reset debounce flag
    sw->debounce_start = false;
}

// Called once per tick (e.g., every second) to update stopwatch counter
void stopwatch_tick(Stopwatch *sw) {
    // Increment seconds
    sw->second_count++;
    // Display only the number for test compatibility
    //printf("%d\n", sw->second_count);
}

void stopwatch_update_state(Stopwatch *sw, bool button_pressed) {
    switch(sw->system_state) {
        case STATE_IDLE:
            // Wait for button press
            if (!button_pressed) {
                // Button pressed -> start debounce process
                sw->system_state = STATE_WAIT_DEBOUNCE;
                sw->debounce_start = true;
            }
            break;

        case STATE_WAIT_DEBOUNCE:
            // Wait until debounce period is cleared
            if (!sw->debounce_start) {
                // If button is confirmed pressed -> move to depressed state
                if (sw->btn_depressed) {
                    sw->system_state = STATE_BTN_DEPRESSED;
                } else {
                    // If false trigger, return to idle
                    sw->system_state = STATE_IDLE;
                }
            }
            break;

        case STATE_BTN_DEPRESSED:
            // Wait for button release
            if (button_pressed) {
                sw->system_state = STATE_BTN_WAIT_RELEASE;
            }
            break;

        case STATE_BTN_WAIT_RELEASE:
            // Reset stopwatch when button is released
            // Reset stopwatch counter
            sw->second_count = 0;
            // Remove or comment out the following line for test compatibility
            // printf("Stopwatch reset to 0 seconds\n");
            // Go back to idle state
            sw->system_state = STATE_IDLE;
            break;

        default:
            sw->system_state = STATE_IDLE;
            break;
    }
}