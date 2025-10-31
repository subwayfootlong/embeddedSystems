#include <stdio.h>
#include "pico/stdlib.h"
#include "wifi_driver.h"
#include "mqtt_driver.h"
#include "secrets.h"

int main(void) {
    stdio_init_all();

    // Wait for serial connection
    sleep_ms(2000);
    printf("=== Pico W MQTT Test ===\n");

    // Initialize server components
    setup_wifi();
    setup_mqtt();

    // Subscribe to topics
    mqtt_subscribe_topic(TOPIC_PUBLISH, 0);

    // Main processing loop
    while (true) {
        mqtt_publish_message(TOPIC_PUBLISH, "hello world", 0, 0);
        sleep_ms(5000);
    }

    // Cleanup (will never reach here in this example)
    return 0;
}
