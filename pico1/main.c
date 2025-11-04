#include <stdio.h>
#include "pico/stdlib.h"
#include "wifi_driver.h"
#include "mqtt_driver.h"
#include "secrets.h"
#include "mq2_driver.h"

int main(void) {
    stdio_init_all();

    // Wait for serial connection
    sleep_ms(2000);
    printf("=== Pico W MQTT Test ===\n");
    
    // Initialize server components
    setup_wifi();
    setup_mqtt();

    // Subscribe to topics
    mqtt_subscribe_topic(TOPIC_SUBSCRIBE_PICO1, 0);

    // Initialize gas sensors
    if (mq2_start() != MQ2_OK) {
        return -1;
    }
    
    uint32_t last_publish_time_ms = 0;

    // Main processing loop
    while (true) {
        cyw43_arch_poll();
        uint32_t current_time_ms = to_ms_since_boot(get_absolute_time());
        const uint32_t sample_interval = mq2_get_config().min_interval_ms;
        
        if (current_time_ms - last_publish_time_ms >= sample_interval) {
            Mq2Reading mq2 = mq2_get_payload();
            
            char payload[128];
            snprintf(payload, sizeof(payload), "LPG(status)=%d, LPG(ppm)=%.2f, LPG(V)=%.2f", mq2.status, mq2.ppm, mq2.voltage);

            mqtt_publish_message(TOPIC_PUBLISH_PICO1, payload, 0, 0);
        }

        last_publish_time_ms = current_time_ms;
        sleep_ms(sample_interval);
    }

    return 0;
}
