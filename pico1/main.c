#include <stdio.h>
#include "pico/stdlib.h"
#include "wifi_driver.h"
#include "mqtt_driver.h"
#include "secrets.h"
#include "mq2_driver.h"
#include "mq7_driver.h"
#include "mq135_driver.h"
#include "pred_driver.h"

int main(void) {
    stdio_init_all();

    // Wait for serial connection
    sleep_ms(2000);
    printf("=== Pico W MQTT Test ===\n");
    
    // Initialize server components
    setup_wifi();
    setup_mqtt();

    // Subscribe to topics
    mqtt_subscribe_topic(TOPIC_PREDICTION, 0);

    // Initialize gas sensors
    mq2_start();
    mq7_init_adc();
    mq135_start();
    
    uint32_t last_publish_time_ms = 0;

    // Main processing loop
    while (true) {
        cyw43_arch_poll();

        uint32_t now = to_ms_since_boot(get_absolute_time());
        uint32_t sample_interval = pred_get_sample_interval();

        if (now - last_publish_time_ms >= sample_interval) {

        mq2_reading mq2 = mq2_get_payload();
        mq7_reading mq7 = mq7_get_payload();
        mq135_reading_t mq135 = mq135_get_payload();

        char payload[128];
        snprintf(payload, sizeof(payload), "%.2f, %.2f, %.2f", mq2.ppm, mq7.ppm, mq135.ppm);

        mqtt_publish_message(TOPIC_PUBLISH_PICO1, payload, 0, 0);

        last_publish_time_ms = now;
        }
        
        sleep_ms(50);
    }

    return 0;
}
