#ifndef PRED_DRIVER_H
#define PRED_DRIVER_H

#include <stdint.h>

typedef enum {
    PRED_NORMAL,
    PRED_WARNING,
    PRED_HIGH
} prediction_level_t;

prediction_level_t pred_get_level(void);
void pred_set_level(prediction_level_t new_pred);

// Get sample interval in milliseconds based on prediction
uint32_t pred_get_sample_interval(void);

#endif // PRED_DRIVER_H