#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdbool.h>

// Stopwatch system states
enum {
    STATE_IDLE,
    STATE_WAIT_DEBOUNCE,
    STATE_BTN_DEPRESSED,
    STATE_BTN_WAIT_RELEASE
};

// Global variables can be managed via a struct
typedef struct {
    int system_state;
    bool btn_depressed;
    bool debounce_start;
    int second_count;
} Stopwatch;

// Initialize the stopwatch
void stopwatch_init(Stopwatch *sw);

// Call on 1-second alarm to update button state
void stopwatch_alarm_update(Stopwatch *sw, bool button_pressed);

// Call on repeating timer to increment seconds
void stopwatch_tick(Stopwatch *sw);

// Update the state machine based on button_pressed
void stopwatch_update_state(Stopwatch *sw, bool button_pressed);

#endif
