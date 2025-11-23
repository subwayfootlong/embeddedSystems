#include "pred_driver.h"
#include <string.h>
#include <stdio.h>

static volatile prediction_level_t g_current_prediction = PRED_NORMAL;

prediction_level_t pred_get_level(void) {
    return g_current_prediction;
}

void pred_set_level(prediction_level_t new_pred) {
    g_current_prediction = new_pred;
}

uint32_t pred_get_sample_interval(void)
{
    switch (pred_get_level()) {
        case PRED_HIGH:
            return 5000;
        case PRED_WARNING:
            return 10000;
        case PRED_NORMAL:
        default:
            return 30000;
    }
}