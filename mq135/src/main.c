#include <stdio.h>
#include "pico/stdlib.h"
#include "mq135_driver.h"

int main() {
    // 1. Initialize standard I/O and wait
    stdio_init_all();
    sleep_ms(1200);
    
    // 2. Run the centralized sensor logic loop
    mq135_run_loop();
    
    return 0; 
}