#include "server_driver.h"
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "wifi_driver.h"
#include "mqtt_driver.h"
#include "sd_driver.h"
#include "timestamp_driver.h"
#include "secrets.h"
#include "lwip/netif.h"
#include "lwip/ip4_addr.h"

// Global SD card manager instance
SD_Manager sd_mgr;

// Integrated message handler
void server_message_handler(const char* topic, const char* payload, uint16_t payload_len) {
    // Debug: Print out every received message
    printf("Received message on topic: %s, payload length: %u\n", topic, payload_len);
    
    // Timestamp synchronization handler
    timestamp_mqtt_handler(topic, payload, payload_len);
    
    // Process sensor data (with synchronized timestamp)
    if (strcmp(topic, TOPIC_PUBLISH) == 0) {
        if (!timestamp_is_synchronized()) {
            printf("Warning: No timestamp received yet\n");
            return;
        }

        // Ensure payload is null-terminated for safe string operations
        char message[payload_len + 1];
        memcpy(message, payload, payload_len);
        message[payload_len] = '\0';
        
        // Get synchronized timestamp
        uint64_t current_timestamp = timestamp_get_synced_time();
        
        // Log message to console
        printf("Message received on %s: %.*s\n", topic, payload_len, payload);
        
        // Prepare CSV entry with timestamp
        char csv_entry[256];
        snprintf(csv_entry, sizeof(csv_entry), "%llu,%s\n", current_timestamp, message);
        
        // Append message to CSV file on SD card
        sd_write_data(&sd_mgr, "sensor_log.csv", csv_entry, true);
    }
}

// Initialize server components
int server_init(void) {
    // Step 1: System Initialization
    stdio_init_all();
   
    // Wait for serial connection
    sleep_ms(2000);
    printf("=== Lutfi Pico Server - MQTT + SD Logger ===\n");
   
    // Initialize SD Card
    if (!sd_init(&sd_mgr)) {
        printf("FATAL: SD card initialization failed!\n");
        return -1;
    }
   
    // Write CSV header (optional)
    sd_write_data(&sd_mgr, "sensor_log.csv", "timestamp,sensor_data\n", false);
   
    // Step 2: Network Connection
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
    if (mqtt_connect(MQTT_BROKER_IP, MQTT_BROKER_PORT, server_message_handler) != MQTT_OK) {
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

    // Step 3: Timestamp Synchronization
    printf("\n5. Requesting timestamp synchronization...\n");
    
    // Initialize timestamp synchronization
    if (!timestamp_init(TOPIC_TIMESTAMP_REQUEST, TOPIC_TIMESTAMP_REPLY)) {
        printf("Timestamp synchronization initialization failed\n");
        return -1;
    }

    // Wait for timestamp with timeout
    timeout = 30;  // Increased timeout to 30 seconds
    while (!timestamp_is_synchronized() && timeout > 0) {
        sleep_ms(1000);
        timeout--;
    }

    if (!timestamp_is_synchronized()) {
        printf("Timestamp synchronization failed\n");
        return -1;
    }

    // Step 4: Subscribe to sensor topics
    if (mqtt_subscribe_topic(TOPIC_PUBLISH, 0) != MQTT_OK) {
        printf("Failed to subscribe to sensor topic\n");
        return -1;
    }

    // Step 5: Final initialization message
    printf("\n6. Server initialized and ready.\n");

    return 0;
}