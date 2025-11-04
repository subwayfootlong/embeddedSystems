#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "acd1100.h"
#include "mqtt_driver.h"
#include "wifi_driver.h"
#include "secrets.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

// ---- Choose I2C instance and pins here ----

int main() {
    // Wait for serial connection
    sleep_ms(2000);

    stdio_init_all();
    acd1100_init(I2C_PORT, I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQ_HZ);

    // Initialize server components
    setup_wifi();
    setup_mqtt();

    // Subscribe to topics
    mqtt_subscribe_topic(TOPIC_SAFETY_LEVEL, 0);

    uint32_t ppm = 0;
    uint16_t t_raw = 0;
    char ppm_payload[16];

    // Main processing loop
    while (true) {
        bool ok = acd1100_read_ppm_string(I2C_PORT, ACD1100_I2C_ADDR, ppm_payload, sizeof ppm_payload, &ppm, &t_raw);

        if (ok) {
            printf("CO2: %lu ppm\n", (unsigned long)ppm);
            mqtt_publish_message(TOPIC_CO2, ppm_payload, 0, 0);
        }
        sleep_ms(5000);
    }
    return 0;
}
