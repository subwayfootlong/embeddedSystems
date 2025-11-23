#include "power_manager.h"
#include "hardware/vreg.h"
#include "hardware/clocks.h"
#include "pico/stdlib.h"

void enter_low_power_mode(void) {
    vreg_set_voltage(VREG_VOLTAGE_1_10);
    sleep_ms(10);
    set_sys_clock_khz(48000, true);
}

void exit_low_power_mode(void) {
    vreg_set_voltage(VREG_VOLTAGE_1_20);
    sleep_ms(10);
    set_sys_clock_khz(125000, true);
}
