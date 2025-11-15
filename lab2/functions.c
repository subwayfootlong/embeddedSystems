#include "pico/stdlib.h"
#include "functions.h"

// LED pin numbers
#define LED0 2
#define LED1 3
#define LED2 4
#define LED3 5

// Initialise LEDs as output pins
void leds_init(void) {
    gpio_init(LED0);
    gpio_set_dir(LED0, GPIO_OUT);
    gpio_init(LED1);
    gpio_set_dir(LED1, GPIO_OUT);
    gpio_init(LED2);
    gpio_set_dir(LED2, GPIO_OUT);
    gpio_init(LED3);
    gpio_set_dir(LED3, GPIO_OUT);
}

// Initialise buttons as input pins with pull-ups
void buttons_init(uint buttonA, uint buttonB) {
    gpio_init(buttonA);
    gpio_set_dir(buttonA, GPIO_IN);
    gpio_pull_up(buttonA);
    gpio_init(buttonB);
    gpio_set_dir(buttonB, GPIO_IN);
    gpio_pull_up(buttonB);
}

// Shift LED pattern left with wrap-around
uint8_t shift_left_wrap(uint8_t leds) {
    // Step 1: shift left
    uint8_t shifted = leds << 1;
    // Step 2: keep only lower 4 bits (mask)
    shifted = shifted & 0b1111;
    // Step 3: check if original MSB (bit 3) was 1
    uint8_t msb = (leds >> 3) & 1;
    // Step 4: add MSB back into LSB position
    uint8_t result = shifted | msb;
    return result;
    
    /* Same thing using if-else:
    if (leds==0b1000){ 
        leds = 0b0001; 
    } 
    else
    { 
        leds = leds << 1; 
    } 
    return leds;
    */ 
}

// Toggle lowest bit (bit0)
uint8_t toggle_lsb(uint8_t leds) {

    // XOR flips the lowest bit
    uint8_t result = leds ^ 0b0001;

    return result;
}

// Write each bit to the physical LED pins
void update_leds(uint8_t leds) {

    uint8_t bit0 = (leds >> 0) & 1;
    uint8_t bit1 = (leds >> 1) & 1;
    uint8_t bit2 = (leds >> 2) & 1;
    uint8_t bit3 = (leds >> 3) & 1;

    gpio_put(LED0, bit0);
    gpio_put(LED1, bit1);
    gpio_put(LED2, bit2);
    gpio_put(LED3, bit3);
}
