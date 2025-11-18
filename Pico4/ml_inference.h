#ifndef ML_INFERENCE_H
#define ML_INFERENCE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

bool ml_inference_init(void);
int ml_inference_run(float lpg, float co, float nh3, float co2);

#ifdef __cplusplus
}
#endif

#endif