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

// Callback for incoming MQTT messages
void mqtt_message_received(const char* topic, const char* payload, uint16_t payload_len) {
    // Ensure payload is null-terminated for safe string operations
    char message[payload_len + 1];
    memcpy(message, payload, payload_len);
    message[payload_len] = '\0';
    
    // Log message to console
    printf("Message received on %s: %.*s\n", topic, payload_len, payload);
    
    // Get system uptime as timestamp
    uint64_t timestamp = to_us_since_boot(get_absolute_time()) / 1000000;
    
    // Prepare CSV entry with timestamp
    char csv_entry[256];
    snprintf(csv_entry, sizeof(csv_entry), "%llu,%s\n", timestamp, message);
    
    // Append message to CSV file on SD card
    sd_write_data(&sd_mgr, "sensor_log.csv", csv_entry, true);
}

int main() {
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
   
    // Subscribe to test/publish topic
    mqtt_subscribe_topic(TOPIC_PUBLISH, 0);
   
    // Main loop
    while (true) {
        // Keep the connection alive
        sleep_ms(5000);
    }
    
    // Cleanup (will never reach here in this example)
    sd_unmount(&sd_mgr);
    return 0;
}