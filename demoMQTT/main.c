#include <stdio.h>
#include "pico/stdlib.h"
#include "wifi_driver.h"
#include "mqtt_driver.h"
#include "secrets.h"
#include "lwip/netif.h"
#include "lwip/ip4_addr.h"

// Callback for incoming MQTT messages
void mqtt_message_received(const char* topic, const char* payload, uint16_t payload_len) {
    printf("Message received: %.*s\n", payload_len, payload);
}

int main() {
    stdio_init_all();
    
    // Wait for serial connection
    sleep_ms(2000);
    printf("=== Pico W MQTT Test ===\n");

    // Initialize & connect WiFi
    printf("\n1. Connecting to WiFi...\n");
    if (wifi_init() != WIFI_OK) return -1;
    if (wifi_connect(WIFI_SSID, WIFI_PASSWORD, 10000) != WIFI_OK) return -1;

    // Wait for IP assignment
    sleep_ms(2000);
    printf("Pico W IP Address: %s\n", ip4addr_ntoa(netif_ip4_addr(netif_default)));
    
    // Initialize MQTT
    printf("\n2. Initializing MQTT...\n");
    if (mqtt_init(MQTT_CLIENT_ID) != MQTT_OK) return -1;
    
    // Connect to MQTT broker
    printf("\n3. Connecting to MQTT broker...\n");
    if (mqtt_connect(MQTT_BROKER_IP, MQTT_BROKER_PORT, mqtt_message_received) != MQTT_OK) {
        return -1;
    }
    
    // Wait for connection
    int timeout = 10;
    while (mqtt_get_status() != MQTT_STATUS_CONNECTED && timeout > 0) {
        sleep_ms(1000);
        timeout--;
    }
    
    if (mqtt_get_status() != MQTT_STATUS_CONNECTED) {
        printf("MQTT connection timeout\n");
        return -1;
    }
    
    printf("\n4. MQTT Connected Successfully!\n");
    
    // Subscribe to test topic
    mqtt_subscribe_topic(TOPIC_TEST_SUBSCRIBE, 0);
    
    // Main loop - publish "hello from pico" every 5 seconds
    while (true) {
        mqtt_publish_message(TOPIC_TEST_PUBLISH, "hello from pico", 0, 0);
        printf("Published: hello from pico\n");
        sleep_ms(5000);
    }

    return 0;
}