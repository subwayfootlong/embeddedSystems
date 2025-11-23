#include "ema_filter.h"
#include <stdbool.h>

static float ema_alpha = 0.25f;   // default smoothing factor
static float ema_prev  = 0.0f;    // previous filtered value
static bool ema_started = false;

void ema_init(float alpha) {
    ema_alpha = alpha;
    ema_started = false;  // reset filter state
}

float ema_process(float new_value) {
    if (!ema_started) {
        ema_prev = new_value;   // first sample → no filtering
        ema_started = true;
        return ema_prev;
    }

    ema_prev = (ema_alpha * new_value) + ((1.0f - ema_alpha) * ema_prev);
    return ema_prev;
}
