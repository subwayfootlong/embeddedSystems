#include <stdio.h>
#include "pico/stdlib.h"
#include "functions.h"

#define BUTTON_A 20
#define BUTTON_B 21

int main() {
    stdio_init_all();

    uint8_t leds = 0b0001;

    leds_init();
    buttons_init(BUTTON_A, BUTTON_B);

    update_leds(leds);

    while (true) {

        if (!gpio_get(BUTTON_A)) {         // button pressed (active low)
            leds = shift_left_wrap(leds);
            update_leds(leds);
            sleep_ms(250);                 // debounce
        }

        if (!gpio_get(BUTTON_B)) {
            leds = toggle_lsb(leds);
            update_leds(leds);
            sleep_ms(250);                 // debounce
        }

        sleep_ms(10); // simple polling delay
    }
}
