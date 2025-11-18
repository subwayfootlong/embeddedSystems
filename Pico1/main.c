#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/vreg.h"
#include "hardware/clocks.h"
#include "wifi_driver.h"
#include "mqtt_driver.h"
#include "acd1100.h"
#include "secrets.h"

#define WIFI_CYCLE_MS      (5 * 60 * 1000)   // every 5 minutes
#define MQTT_SETTLE_MS     500               // small delay for broker response
#define LOW_CLOCK_KHZ      48000             // 48 MHz for low power
#define HIGH_CLOCK_KHZ     125000            // 125 MHz for performance

int main() {
    stdio_init_all();
    sleep_ms(2000);
    
    gpio_init(0);
    gpio_set_dir(0, GPIO_OUT);
    gpio_put(0, 1);
    gpio_init(1);
    gpio_set_dir(1, GPIO_OUT);
    gpio_put(1, 1);

    printf("=== Pico W CO2 Monitor (Voltage & Regulator Control) ===\n");

    // Initialize CO₂ sensor once at startup
    acd1100_init(I2C_PORT, I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQ_HZ);

    uint32_t ppm = 0;
    uint16_t t_raw = 0;
    char ppm_payload[16];

    while (true) {
        // === STEP 1: Boost performance for network & sensing ===
        printf("\n[High Performance] Restoring 1.2V @ 125MHz\n");
        vreg_set_voltage(VREG_VOLTAGE_1_20);
        sleep_ms(10);
        set_sys_clock_khz(HIGH_CLOCK_KHZ, true);

        setup_wifi();
        setup_mqtt();
        mqtt_subscribe_topic(TOPIC_SAFETY_LEVEL, 0);

        bool ok = acd1100_read_ppm_string(I2C_PORT, ACD1100_I2C_ADDR,
                                          ppm_payload, sizeof(ppm_payload),
                                          &ppm, &t_raw);

        if (ok) {
            printf("CO2 Reading: %lu ppm\n", (unsigned long)ppm);
            mqtt_publish_message(TOPIC_CO2, ppm_payload, 0, 0);
        } else {
            printf("Sensor read failed.\n");
        }

        sleep_ms(MQTT_SETTLE_MS);

        mqtt_disconnect_client();
        wifi_deinit();

        // === STEP 2: Drop voltage & clock for idle period ===
        printf("[Low Power] Switching to 1.1V @ 48MHz for %d ms...\n", WIFI_CYCLE_MS);
        vreg_set_voltage(VREG_VOLTAGE_1_10);
        sleep_ms(10);
        set_sys_clock_khz(LOW_CLOCK_KHZ, true);

        // Stay in low-power idle for 5 minutes
        sleep_ms(WIFI_CYCLE_MS);
    }

    return 0;
}
