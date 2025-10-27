#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "wifi_driver.h"
#include "mqtt_driver.h"
#include "sd_driver.h"
#include "secrets.h"
#include "lwip/netif.h"
#include "lwip/ip4_addr.h"

// Global SD card manager instance
SD_Manager sd_mgr;

// Global variable to store initial timestamp
static uint64_t initial_pc_timestamp = 0;
static bool timestamp_received = false;

// Callback for incoming MQTT messages
void mqtt_message_received(const char* topic, const char* payload, uint16_t payload_len) {
    // Debug: Print out every received message
    printf("Received message on topic: %s, payload length: %u\n", topic, payload_len);
    
    // Timestamp synchronization response
    if (strcmp(topic, TOPIC_TIMESTAMP_REPLY) == 0 && !timestamp_received) {
        // Ensure payload is null-terminated for safe string operations
        char raw_payload[32];
        memset(raw_payload, 0, sizeof(raw_payload));
        memcpy(raw_payload, payload, payload_len);
        printf("Raw timestamp payload: %s\n", raw_payload);

        // Convert payload to uint64_t
        char* endptr;
        initial_pc_timestamp = strtoull(raw_payload, &endptr, 10);
        
        // Check if conversion was successful
        if (endptr != raw_payload) {
            timestamp_received = true;
            
            // Unsubscribe from timestamp reply topic
            mqtt_unsubscribe_topic(TOPIC_TIMESTAMP_REPLY);
            
            printf("Timestamp synchronized successfully: %llu\n", initial_pc_timestamp);
            return;
        } else {
            printf("Failed to parse timestamp: %s\n", raw_payload);
        }
    }
    
    // Process sensor data (with synchronized timestamp)
    if (strcmp(topic, TOPIC_PUBLISH) == 0) {
        if (!timestamp_received) {
            printf("Warning: No timestamp received yet\n");
            return;
        }

        // Ensure payload is null-terminated for safe string operations
        char message[payload_len + 1];
        memcpy(message, payload, payload_len);
        message[payload_len] = '\0';
        
        // Calculate current timestamp
        uint64_t local_uptime = to_us_since_boot(get_absolute_time()) / 1000;
        uint64_t current_timestamp = initial_pc_timestamp + local_uptime;
        
        // Log message to console
        printf("Message received on %s: %.*s\n", topic, payload_len, payload);
        
        // Prepare CSV entry with timestamp
        char csv_entry[256];
        snprintf(csv_entry, sizeof(csv_entry), "%llu,%s\n", current_timestamp, message);
        
        // Append message to CSV file on SD card
        sd_write_data(&sd_mgr, "sensor_log.csv", csv_entry, true);
    }
}

int main() {
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

    // Step 3: Timestamp Synchronization
    printf("\n5. Requesting timestamp synchronization...\n");
    
    // Subscribe to the timestamp reply topic FIRST
    if (mqtt_subscribe_topic(TOPIC_TIMESTAMP_REPLY, 0) != MQTT_OK) {
        printf("Failed to subscribe to timestamp reply topic\n");
        return -1;
    }

    // Publish a timestamp request
    if (mqtt_publish_message(TOPIC_TIMESTAMP_REQUEST, "request", 0, false) != MQTT_OK) {
        printf("Failed to publish timestamp request\n");
        return -1;
    }

    // Wait for timestamp with timeout
    timeout = 30;  // Increased timeout to 30 seconds
    while (!timestamp_received && timeout > 0) {
        sleep_ms(1000);
        timeout--;
    }

    if (!timestamp_received) {
        printf("Timestamp synchronization failed\n");
        return -1;
    }

    // Step 4: Subscribe to sensor topics
    if (mqtt_subscribe_topic(TOPIC_PUBLISH, 0) != MQTT_OK) {
        printf("Failed to subscribe to sensor topic\n");
        return -1;
    }
   
    // Step 5: Main Processing Loop
    printf("\n6. Starting main processing loop...\n");
    while (true) {
        // Keep the connection alive and process any pending messages
        sleep_ms(5000);
    }
   
    // Cleanup (will never reach here in this example)
    sd_unmount(&sd_mgr);
    return 0;
}