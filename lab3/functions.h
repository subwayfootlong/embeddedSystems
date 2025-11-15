#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdbool.h>
#include <stdint.h>
#include "pico/time.h"

typedef enum {
    IDLE,
    DEBOUNCE,
    RUNNING
} StopwatchState;

typedef struct {
    StopwatchState state;
    int seconds;
    bool debounce_active;
} Stopwatch;

void sw_init(Stopwatch *sw);
void sw_tick(Stopwatch *sw);
int64_t sw_debounce_cb(alarm_id_t id, void *user_data);
void sw_update(Stopwatch *sw, bool raw);

#endif
