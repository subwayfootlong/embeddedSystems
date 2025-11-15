#include "functions.h"
#include "hardware/gpio.h"
#include <stdio.h>

#define BTN_PIN 21 // active-low

void sw_init(Stopwatch *sw)
{
    sw->state = IDLE;
    sw->seconds = 0;
    sw->debounce_active = false;
}

// 1-second interrupt
void sw_tick(Stopwatch *sw)
{
    if (sw->state == RUNNING)
    {
        sw->seconds++;
        printf("%d\n", sw->seconds);
    }
}

// debounce after 50 ms
int64_t sw_debounce_cb(alarm_id_t id, void *user_data)
{
    Stopwatch *sw = (Stopwatch *)user_data;
    sw->debounce_active = false;

    bool pressed = !gpio_get(BTN_PIN);

    if (pressed && sw->state == DEBOUNCE)
    {
        sw->seconds = 0; // start fresh
        printf("0\n");
        sw->state = RUNNING;
    }
    else
    {
        sw->state = IDLE;
    }
    return 0;
}

void sw_update(Stopwatch *sw, bool raw)
{
    bool pressed = !raw; // active-low

    switch (sw->state)
    {
    case IDLE:
        if (pressed && !sw->debounce_active)
        {
            sw->debounce_active = true;
            sw->state = DEBOUNCE;
            add_alarm_in_ms(50, sw_debounce_cb, sw, false);
        }
        break;

    case RUNNING:
        if (!pressed)
        {
            sw->state = IDLE;
            sw->seconds = 0; // reset only, no print
        }
        break;

    case DEBOUNCE:
    default:
        break;
    }
}
