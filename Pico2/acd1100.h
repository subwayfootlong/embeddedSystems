#ifndef ACD1100_H
#define ACD1100_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "hardware/i2c.h"

#define ACD1100_I2C_ADDR 0x2A
#define ACD1100_REQUEST_DELAY_MS 100u

#ifndef ACD1100_DEFAULT_I2C
#define ACD1100_DEFAULT_I2C i2c1
#endif

#ifndef ACD1100_DEFAULT_SDA_PIN
#define ACD1100_DEFAULT_SDA_PIN 26u
#endif

#ifndef ACD1100_DEFAULT_SCL_PIN
#define ACD1100_DEFAULT_SCL_PIN 27u
#endif

#ifndef ACD1100_DEFAULT_FREQ_HZ
#define ACD1100_DEFAULT_FREQ_HZ 100000u
#endif

#define I2C_PORT           i2c1
#define I2C_SDA_PIN        26      
#define I2C_SCL_PIN        27      
#define I2C_FREQ_HZ        100000 // 100 kHz
#define ACD1100_I2C_ADDR   0x2A

#define ACD1100_PIN_DEFAULT ((uint)(-1))

typedef enum {
    ACD1100_OK          = 0,
    ACD1100_ERR_INVAL   = -1,
    ACD1100_ERR_I2C     = -2,
    ACD1100_ERR_CRC     = -3,
    ACD1100_ERR_RANGE   = -4,
    ACD1100_ERR_FORMAT  = -5
} acd1100_status_t;

bool acd1100_read_measurement(i2c_inst_t *i2c,
                              uint8_t address,
                              uint32_t *ppm_out,
                              uint16_t *temp_raw_out);

size_t acd1100_format_ppm(char *buf,
                          size_t buf_len,
                          uint32_t ppm);

acd1100_status_t acd1100_read_ppm_string(
    i2c_inst_t *i2c,
    uint8_t address,
    char *ppm_str,
    size_t ppm_str_len,
    uint32_t *ppm_out,
    uint16_t *t_raw_out
);

void acd1100_init(i2c_inst_t *i2c,
                  uint sda_pin,
                  uint scl_pin,
                  uint32_t freq_hz);

void acd1100_run_loop(i2c_inst_t *i2c,
                      uint8_t address,
                      uint32_t print_interval_ms);

bool read_and_publish_ppm(void);

#endif
