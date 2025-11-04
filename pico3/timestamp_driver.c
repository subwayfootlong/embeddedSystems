#include "timestamp_driver.h"
#include "mqtt_driver.h"
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"

// Static variables for timestamp management
static uint64_t initial_pc_timestamp = 0;
static bool timestamp_received = false;
static char request_topic[64] = {0};
static char reply_topic[64] = {0};

// Initialize timestamp synchronization
bool timestamp_init(const char* req_topic, const char* rep_topic) {
    // Clear previous state
    timestamp_received = false;
    initial_pc_timestamp = 0;

    // Store topics
    strncpy(request_topic, req_topic, sizeof(request_topic) - 1);
    strncpy(reply_topic, rep_topic, sizeof(reply_topic) - 1);

    // Subscribe to reply topic
    if (mqtt_subscribe_topic(reply_topic, 0) != MQTT_OK) {
        printf("Failed to subscribe to timestamp reply topic\n");
        return false;
    }

    printf("Timestamp synchronization initialized\n");
    return true;
}

// Request timestamp synchronization
bool timestamp_request_sync(void) {
    if (mqtt_publish_message(request_topic, "request", 0, false) != MQTT_OK) {
        printf("Failed to publish timestamp request\n");
        return false;
    }
    
    printf("Timestamp synchronization requested\n");
    return true;
}

// Wait for timestamp synchronization with timeout
bool timestamp_wait_sync(uint32_t timeout_ms) {
    uint32_t start_time = to_ms_since_boot(get_absolute_time());
    
    while (!timestamp_received) {
        if (to_ms_since_boot(get_absolute_time()) - start_time > timeout_ms) {
            printf("Timestamp synchronization timeout\n");
            return false;
        }
        sleep_ms(100);
    }
    
    return true;
}

// MQTT message handler for timestamp synchronization
void timestamp_mqtt_handler(const char* topic, const char* payload, uint16_t payload_len) {
    // Ensure we only process reply topic and haven't already received timestamp
    if (strcmp(topic, reply_topic) != 0 || timestamp_received) {
        return;
    }

    // Ensure payload is null-terminated for safe string operations
    char raw_payload[32];
    memset(raw_payload, 0, sizeof(raw_payload));
    
    if (payload_len >= sizeof(raw_payload)) {
        printf("Timestamp payload too long: %u bytes\n", payload_len);
        return;
    }
    
    memcpy(raw_payload, payload, payload_len);
    printf("Raw timestamp payload: %s\n", raw_payload);

    // Convert payload to uint64_t
    char* endptr;
    initial_pc_timestamp = strtoull(raw_payload, &endptr, 10);
    
    // Check if conversion was successful
    if (endptr != raw_payload) {
        timestamp_received = true;
        
        // Unsubscribe from timestamp reply topic since we only need it once
        mqtt_unsubscribe_topic(reply_topic);
        
        printf("Timestamp synchronized successfully: %llu\n", initial_pc_timestamp);
    } else {
        printf("Failed to parse timestamp: %s\n", raw_payload);
    }
}

// Check if timestamp is synchronized
bool timestamp_is_synchronized(void) {
    return timestamp_received;
}

// Get synchronized time
uint64_t timestamp_get_synced_time(void) {
    if (!timestamp_received) {
        return 0;
    }

    // Calculate current timestamp (convert microseconds to milliseconds)
    uint64_t local_uptime = to_us_since_boot(get_absolute_time()) / 1000;
    return initial_pc_timestamp + local_uptime;
}

// Reset timestamp synchronization
void timestamp_reset_sync(void) {
    timestamp_received = false;
    initial_pc_timestamp = 0;
    printf("Timestamp synchronization reset\n");
}