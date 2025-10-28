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

// Sensor data handler (extracted from main message handler)
static void handle_sensor_data(const char* topic, const char* payload, uint16_t payload_len) {
    if (!timestamp_is_synchronized()) {
        printf("Warning: No timestamp received yet\n");
        return;
    }

    // Safe buffer with bounds checking
    char message[256];
    if (payload_len >= sizeof(message)) {
        printf("Payload too large: %u bytes\n", payload_len);
        return;
    }
    
    memcpy(message, payload, payload_len);
    message[payload_len] = '\0';
    
    // Get synchronized timestamp
    uint64_t current_timestamp = timestamp_get_synced_time();
    
    // Log message to console
    printf("Sensor data received: %s\n", message);
    
    // Prepare CSV entry with timestamp
    char csv_entry[256];
    snprintf(csv_entry, sizeof(csv_entry), "%llu,%s\n", current_timestamp, message);
    
    // Append message to CSV file on SD card
    sd_write_data(&sd_mgr, "sensor_log.csv", csv_entry, true);
}

// Integrated message handler - now acts as a router
void server_message_handler(const char* topic, const char* payload, uint16_t payload_len) {
    // Debug: Print out every received message
    printf("Received message on topic: %s, payload length: %u\n", topic, payload_len);
    
    // Route messages to appropriate handlers
    if (strcmp(topic, TOPIC_TIMESTAMP_REPLY) == 0) {
        timestamp_mqtt_handler(topic, payload, payload_len);
    } else if (strcmp(topic, TOPIC_PUBLISH) == 0) {
        handle_sensor_data(topic, payload, payload_len);
    } else {
        printf("Unknown topic: %s\n", topic);
    }
}

// Check if all system components are ready
static bool is_system_ready(void) {
    return (wifi_is_connected() && 
            mqtt_get_status() == MQTT_STATUS_CONNECTED &&
            timestamp_is_synchronized());
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
   
    // Initialize CSV log file with header
    if (!sd_init_csv_log(&sd_mgr, "sensor_log.csv")) {
        printf("Warning: Failed to initialize CSV log file\n");
    }
   
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
   
    // Wait for connection using new helper function
    if (!mqtt_wait_connection(10000)) {
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

    // Request sync and wait
    if (!timestamp_request_sync()) {
        printf("Failed to request timestamp synchronization\n");
        return -1;
    }

    // Wait for timestamp with timeout
    if (!timestamp_wait_sync(30000)) {
        printf("Timestamp synchronization failed - timeout\n");
        return -1;
    }

    // Step 4: Subscribe to sensor topics
    if (mqtt_subscribe_topic(TOPIC_PUBLISH, 0) != MQTT_OK) {
        printf("Failed to subscribe to sensor topic\n");
        return -1;
    }

    // Step 5: Final initialization message
    printf("\n6. Server initialized and ready.\n");
    printf("System status: WiFi=%s, MQTT=%s, Timestamp=%s\n",
           wifi_is_connected() ? "Connected" : "Disconnected",
           mqtt_get_status() == MQTT_STATUS_CONNECTED ? "Connected" : "Disconnected",
           timestamp_is_synchronized() ? "Synced" : "Not Synced");

    return 0;
}