#include <stdio.h>
#include "pico/stdlib.h"
#include "mq135_driver.h"

int main() {
    stdio_init_all();
    sleep_ms(1200);
    
    mq135_print_config();
    mq135_setup();
    
    mq135_reading_t reading;
    
    while (true) {
        mq135_read(&reading);
        mq135_print(&reading);
        sleep_ms(1000);
    }
    
    return 0;
}