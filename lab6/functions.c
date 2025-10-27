#include "functions.h"

float compute_pid(float setpoint, float current_value, float *integral, float *prev_error,
                  float Kp, float Ki, float Kd) {

    // 1. Compute current error
    float error = setpoint - current_value;

    // 2. Update the integral term (accumulate total error)
    *integral += error;

    // 3. Compute the derivative term (difference in error)
    float derivative = error - *prev_error;

    // 4. Compute the PID control signal
    float control_signal = (Kp * error) + (Ki * (*integral)) + (Kd * derivative);

    // 5. Update the previous error for next iteration
    *prev_error = error;

    // 6. Return the control signal
    return control_signal;
}
