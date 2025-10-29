#include "acd1100.h"
#include <stdio.h>
#include "hardware/i2c.h"
#include "pico/stdlib.h"

static uint8_t crc8_poly31_initFF(const uint8_t *data, size_t len) {
    uint8_t crc = 0xFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int b = 0; b < 8; b++) {
            if (crc & 0x80) {
                crc = (uint8_t)((crc << 1) ^ 0x31);
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

// Read measurements from acd1100
bool acd1100_read_measurement(i2c_inst_t *i2c,
                              uint8_t address,
                              uint32_t *ppm_out,
                              uint16_t *temp_raw_out) {
    if (!i2c) {
        return false;
    }

    const uint8_t cmd[2] = {0x03, 0x00};
    int written = i2c_write_blocking(i2c, address, cmd, 2, false);
    if (written != 2) {
        return false;
    }

    sleep_ms(ACD1100_REQUEST_DELAY_MS);

    uint8_t buf[9] = {0};
    int read = i2c_read_blocking(i2c, address, buf, 9, false);
    if (read != 9) {
        return false;
    }

    const uint8_t ppm_hi[2] = {buf[0], buf[1]};
    const uint8_t crc1 = buf[2];
    const uint8_t ppm_lo[2] = {buf[3], buf[4]};
    const uint8_t crc2 = buf[5];
    const uint8_t t_raw[2] = {buf[6], buf[7]};
    const uint8_t crc3 = buf[8];

    if (crc8_poly31_initFF(ppm_hi, 2) != crc1) {
        return false;
    }
    if (crc8_poly31_initFF(ppm_lo, 2) != crc2) {
        return false;
    }
    if (crc8_poly31_initFF(t_raw, 2) != crc3) {
        return false;
    }

    if (ppm_out) {
        *ppm_out = ((uint32_t)ppm_hi[0] << 24) |
                   ((uint32_t)ppm_hi[1] << 16) |
                   ((uint32_t)ppm_lo[0] << 8) |
                   ((uint32_t)ppm_lo[1]);
    }

    if (temp_raw_out) {
        *temp_raw_out = ((uint16_t)t_raw[0] << 8) | t_raw[1];
    }

    return true;
}

void acd1100_init(i2c_inst_t *i2c,
                  uint sda_pin,
                  uint scl_pin,
                  uint32_t freq_hz) {
    if (!i2c) {
        return;
    }

    i2c_init(i2c, freq_hz);
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);

    printf("ACD1100 CO2 reader (I2C addr 0x%02X)\n", ACD1100_I2C_ADDR);
}

void acd1100_run_loop(i2c_inst_t *i2c,
                      uint8_t address,
                      uint32_t print_interval_ms) {
    if (!i2c) {
        return;
    }

    const uint32_t poll_delay_ms = 200;
    absolute_time_t last_print = get_absolute_time();
    uint32_t ppm = 0;
    uint16_t t_raw = 0;

    while (true) {
        
        bool ok = acd1100_read_measurement(i2c, address, &ppm, &t_raw);

        absolute_time_t now = get_absolute_time();
        if (to_ms_since_boot(now) - to_ms_since_boot(last_print) >= print_interval_ms) {
            if (ok) {
                printf("CO2: %lu ppm\n", (unsigned long)ppm);
            } else {
                printf("Read error (I2C/CRC)\n");
            }
            last_print = now;
        }

        sleep_ms(poll_delay_ms);
    }
}
