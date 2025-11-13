#include <stdio.h>
#include "pico/stdlib.h"


int main() {

    uint a = 1; // initial delay
    const uint LED_PIN = 28;

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    while (true) {
        gpio_put(LED_PIN, 1);
        sleep_ms(a);
        gpio_put(LED_PIN, 0);
        sleep_ms(a);

        // Double the delay
        a = a << 1;   // same as a *= 2

        // Reset back to 1 when reaching 2048
        if (a == 2048) {
            a = 1;
        }
    }

}
