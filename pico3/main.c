#include "pico3_driver.h"
#include "pico/stdlib.h"

int main(void) {
    if (pico3_driver_init() != 0) {
        return -1;
    }
    while (true) {
        sleep_ms(5000);
    }
}
