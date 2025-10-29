#ifndef ACD1100_H
#define ACD1100_H

#include <stdbool.h>
#include <stdint.h>
#include "hardware/i2c.h"

#define ACD1100_I2C_ADDR 0x2A
#define ACD1100_REQUEST_DELAY_MS 100u

bool acd1100_read_measurement(i2c_inst_t *i2c,
                              uint8_t address,
                              uint32_t *ppm_out,
                              uint16_t *temp_raw_out);

void acd1100_init(i2c_inst_t *i2c,
                  uint sda_pin,
                  uint scl_pin,
                  uint32_t freq_hz);

void acd1100_run_loop(i2c_inst_t *i2c,
                      uint8_t address,
                      uint32_t print_interval_ms);

#endif
