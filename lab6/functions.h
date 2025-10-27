#ifndef FUNCTIONS_H
#define FUNCTIONS_H

// ============================================================
// functions.h
// Header file declaring the prototype for the PID computation function
// ============================================================

// Function: compute_pid
// ---------------------
// Calculates the PID control signal based on the given parameters.
//
// Parameters:
//   setpoint       - The desired target value the system should reach
//   current_value  - The current measured value of the system
//   *integral      - Pointer to the accumulated integral term (updated each call)
//   *prev_error    - Pointer to the previous error (used for derivative calculation)
//   Kp             - Proportional gain constant
//   Ki             - Integral gain constant
//   Kd             - Derivative gain constant
//   dt             - Time step (seconds) used for integral and derivative calculations
//
// Returns:
//   The computed PID control output (float), which can be used to adjust
//   the system actuator (e.g., motor speed or valve position).
//
float compute_pid(float setpoint, float current_value, float *integral, float *prev_error,
                  float Kp, float Ki, float Kd, float dt);

#endif // FUNCTIONS_H
