#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "functions.h"

#define BTN_PIN 21

Stopwatch sw;

bool timer_cb(struct repeating_timer *t) {
    sw_tick(&sw);
    return true;
}

int main() {
    stdio_init_all();
    sleep_ms(3000);

    gpio_init(BTN_PIN);
    gpio_set_dir(BTN_PIN, GPIO_IN);
    gpio_set_pulls(BTN_PIN, true, false);

    sw_init(&sw);

    struct repeating_timer timer;
    add_repeating_timer_ms(1000, timer_cb, NULL, &timer);

    printf("Stopwatch ready. Hold button to start.\n");

    while (1) {
        sw_update(&sw, gpio_get(BTN_PIN));
        sleep_ms(10);
    }
}
