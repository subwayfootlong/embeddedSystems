#ifndef ML_INFERENCE_H
#define ML_INFERENCE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

// Initialise TFLM (must be called once at startup).
bool ml_inference_init(void);

// Run inference on one sample.
// Inputs are the 4 sensor values (raw or pre-scaled, see comment in .cpp).
// Returns:
//   0 = normal
//   1 = medium
//   2 = high
//  -1 = error (interpreter not ready or invoke failed)
int ml_inference_run(float lpg, float co, float nh3, float co2);

#ifdef __cplusplus
}
#endif

#endif // ML_INFERENCE_H
