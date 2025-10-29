#include <stdio.h>
#include "pico/stdlib.h"
#include "MQ7Functions.h"


int main() {
    stdio_init_all();
    sleep_ms(1200); // give USB-CDC time

    mq7_init_adc();

    printf("\nStarting constant 3.3V fast reading cycle (Reading every ~1.5 seconds)...\n");

    while (true) {
        mq7_read_and_print_stats(); 

        sleep_ms(1500); // Wait 1.5 seconds before the next reading
    }
    return 0;
}