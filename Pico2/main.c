#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/vreg.h"
#include "hardware/clocks.h"
#include "pico/cyw43_arch.h"

#include "acd1100.h"
#include "ema_filter.h"
#include "mqtt_driver.h"
#include "wifi_driver.h"
#include "power_manager.h"
#include "secrets.h"

static const uint32_t INTERVALS[] = {
    INTERVAL_NORMAL,
    INTERVAL_WARNING,
    INTERVAL_HIGH
};

volatile int safety_level = 0;

int main() {
    stdio_init_all();
    sleep_ms(1500);

    acd1100_init(I2C_PORT, I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQ_HZ);
    ema_init(0.25f);

    setup_wifi();
    setup_mqtt();
    mqtt_subscribe_topic(TOPIC_SAFETY_LEVEL, 0);
    listen_for_mqtt_updates(1000);

    while (true) {

        read_and_publish_ppm();
        sleep_ms(1000);
        cyw43_arch_poll();

        uint32_t interval_ms = INTERVALS[safety_level];

        if (safety_level == 0) {

            printf("[NORMAL] Low-power sleep %u ms\n", interval_ms);

            mqtt_disconnect_client();
            wifi_deinit();

            enter_low_power_mode();
            sleep_ms(interval_ms);
            exit_low_power_mode();

            // Reconnect fully once
            setup_wifi();
            setup_mqtt();
            mqtt_subscribe_topic(TOPIC_SAFETY_LEVEL, 0);
            listen_for_mqtt_updates(2000);
        }

        else {
            printf("[ALERT MODE] Staying awake for %u ms\n", interval_ms);

            listen_for_mqtt_updates(2000);

            sleep_ms(interval_ms);
        }
    }

    return 0;
}
