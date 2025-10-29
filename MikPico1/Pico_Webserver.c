#include "pico/stdlib.h"
#include <stdio.h>
#include <stdlib.h>
#include "lwip/err.h"
#include "pico/cyw43_arch.h"
#include "secrets.h"
#include "webserver.h"
#include "wifi_driver.h"
#include "acd1100.h"

#define I2C_PORT i2c1
#define I2C_SDA_PIN 26
#define I2C_SCL_PIN 27
#define I2C_FREQ_HZ 100000
#define SENSOR_POLL_INTERVAL_MS 1000

#define HTTP_PORT 80
#define WIFI_CONNECT_TIMEOUT_MS 30000

int main(void) {
    stdio_init_all();
    srand(to_ms_since_boot(get_absolute_time()));

    if (wifi_init() != WIFI_OK) {
        return 1;
    }

    if (wifi_connect(WIFI_SSID, WIFI_PASSWORD, WIFI_CONNECT_TIMEOUT_MS) != WIFI_OK) {
        wifi_deinit();
        return 1;
    }

    if (webserver_start(HTTP_PORT) != ERR_OK) {
        printf("Webserver start failed\n");
        wifi_deinit();
        return 1;
    }

    acd1100_init(I2C_PORT, I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQ_HZ);
    absolute_time_t last_sensor_poll = get_absolute_time();

    uint32_t ppm = 0;
    uint16_t temp_raw = 0;

    while (true) {
        absolute_time_t now = get_absolute_time();
        if (to_ms_since_boot(now) - to_ms_since_boot(last_sensor_poll) >= SENSOR_POLL_INTERVAL_MS) {
            bool ok = acd1100_read_measurement(I2C_PORT, ACD1100_I2C_ADDR, &ppm, &temp_raw);
            if (ok) {
                printf("ACD1100 ppm: %lu (temp raw: 0x%04X)\n",
                       (unsigned long)ppm, temp_raw);
                webserver_set_acd_reading(ppm, true);
            } else {
                printf("ACD1100 read failed\n");
                webserver_set_acd_reading(0, false);
            }
            last_sensor_poll = now;
        }

        cyw43_arch_poll();
        sleep_ms(10);
    }

    webserver_stop();
    wifi_deinit();
    return 0;
}
