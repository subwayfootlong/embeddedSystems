#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdint.h>

void leds_init(void);
void buttons_init(uint buttonA, uint buttonB);

uint8_t shift_left_wrap(uint8_t leds);
uint8_t toggle_lsb(uint8_t leds);
void update_leds(uint8_t leds);

#endif
