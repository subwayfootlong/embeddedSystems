#include <stdio.h>
#include <stdlib.h>
#include <math.h>
// #include <unistd.h>     // Removed - use Pico SDK sleep instead

#include "functions.h"  // Contains compute_pid() prototype
#include <pico/stdlib.h> // pico stdio + sleep_ms/sleep_us

// PID controller constants
float Kp = 2.0;
float Ki = 0.2;
float Kd = 0.02;

int main() {
    stdio_init_all(); // initialize stdio for printf over UART/USB

    float setpoint = 100.0;     // Desired target position
    float current_value = 0.0;  // Current measured position
    float integral = 0.0;       // Accumulated integral term
    float prev_error = 0.0;     // Previous error for derivative term

    float time_step = 0.1;      // Simulation time step (in seconds)
    int num_iterations = 100;   // Number of control loop iterations

    // Simulate the control loop
    for (int i = 0; i < num_iterations; i++) {
        // Pass Kp, Ki, Kd and dt into compute_pid
        float control_signal = compute_pid(setpoint, current_value, &integral, &prev_error,
                                           Kp, Ki, Kd, time_step);

        // Simulated system response — apply control to update current position
        current_value += control_signal * 0.05f;

        // Display results for each iteration
        printf("Iteration %d: Control Signal = %.2f, Current Position = %.2f\n",
               i, control_signal, current_value);

        // Sleep using Pico SDK (milliseconds)
        sleep_ms((uint32_t)(time_step * 1000.0f));
    }

    return 0;
}
