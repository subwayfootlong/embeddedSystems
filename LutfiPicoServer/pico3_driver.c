#include "pico3_driver.h"
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"

#include "wifi_driver.h"
#include "mqtt_driver.h"
#include "sd_driver.h"
#include "timestamp_driver.h"
#include "http_server_driver.h"
#include "secrets.h"

#include "lwip/netif.h"
#include "lwip/ip4_addr.h"

// Global SD card manager instance
static SD_Manager sd_mgr;

/* ==========================================================
   Sensor data handler
   ========================================================== */
static void handle_sensor_data(const char* topic, const char* payload, uint16_t payload_len) {
    if (!timestamp_is_synchronized()) {
        printf("Warning: No timestamp received yet\n");
        return;
    }

    if (payload_len >= 256) {
        printf("Payload too large: %u bytes\n", payload_len);
        return;
    }

    char message[256];
    memcpy(message, payload, payload_len);
    message[payload_len] = '\0';

    uint64_t current_timestamp = timestamp_get_synced_time();
    printf("Sensor data received: %s\n", message);

    char csv_entry[256];
    snprintf(csv_entry, sizeof(csv_entry), "%llu,%s\n", current_timestamp, message);
    sd_write_data(&sd_mgr, "sensor_log.csv", csv_entry, true);
}

/* ==========================================================
   Unified MQTT message handler
   ========================================================== */
static void pico3_message_handler(const char* topic, const char* payload, uint16_t payload_len) {
    printf("Received message on topic: %s, length: %u\n", topic, payload_len);

    if (strcmp(topic, TOPIC_TIMESTAMP_REPLY) == 0) {
        timestamp_mqtt_handler(topic, payload, payload_len);
    } else if (strcmp(topic, TOPIC_PUBLISH) == 0) {
        handle_sensor_data(topic, payload, payload_len);
    } else {
        printf("Unknown topic: %s\n", topic);
    }
}

/* ==========================================================
   System readiness check
   ========================================================== */
static bool is_system_ready(void) {
    return (wifi_is_connected() &&
            mqtt_get_status() == MQTT_STATUS_CONNECTED &&
            timestamp_is_synchronized());
}

/* ==========================================================
   Main initialization sequence
   ========================================================== */
int pico3_driver_init(void) {
    stdio_init_all();
    sleep_ms(2000);

    printf("=== Lutfi Pico Server - MQTT + SD Logger + HTTP ===\n");

    /* --- Step 1: SD card --- */
    if (!sd_init(&sd_mgr)) {
        printf("FATAL: SD card initialization failed!\n");
        return -1;
    }

    if (!sd_init_csv_log(&sd_mgr, "sensor_log.csv")) {
        printf("Warning: Failed to initialize CSV log file\n");
    }

    /* --- Step 2: Wi-Fi --- */
    printf("\n1. Connecting to WiFi...\n");
    if (wifi_init() != WIFI_OK) return -1;
    if (wifi_connect(WIFI_SSID, WIFI_PASSWORD, 10000) != WIFI_OK) return -1;

    sleep_ms(2000);
    printf("Pico W IP Address: %s\n", ip4addr_ntoa(netif_ip4_addr(netif_default)));

    /* --- Step 3: MQTT --- */
    printf("\n2. Initializing MQTT...\n");
    if (mqtt_init(MQTT_CLIENT_ID) != MQTT_OK) return -1;

    printf("\n3. Connecting to MQTT broker...\n");
    if (mqtt_connect(MQTT_BROKER_IP, MQTT_BROKER_PORT, pico3_message_handler) != MQTT_OK)
        return -1;

    if (!mqtt_wait_connection(10000)) {
        printf("MQTT connection timeout\n");
        return -1;
    }

    printf("\n4. MQTT Connected Successfully!\n");

    /* --- Step 4: Timestamp sync --- */
    printf("\n5. Requesting timestamp synchronization...\n");
    if (!timestamp_init(TOPIC_TIMESTAMP_REQUEST, TOPIC_TIMESTAMP_REPLY)) {
        printf("Timestamp sync initialization failed\n");
        return -1;
    }

    if (!timestamp_request_sync()) {
        printf("Failed to request timestamp\n");
        return -1;
    }

    if (!timestamp_wait_sync(30000)) {
        printf("Timestamp sync timeout\n");
        return -1;
    }

    /* --- Step 5: MQTT subscriptions --- */
    if (mqtt_subscribe_topic(TOPIC_PUBLISH, 0) != MQTT_OK) {
        printf("Failed to subscribe to sensor topic\n");
        return -1;
    }

    printf("\n6. System initialized and ready.\n");
    printf("Status: WiFi=%s, MQTT=%s, Timestamp=%s\n",
           wifi_is_connected() ? "Connected" : "Disconnected",
           mqtt_get_status() == MQTT_STATUS_CONNECTED ? "Connected" : "Disconnected",
           timestamp_is_synchronized() ? "Synced" : "Not Synced");

    /* --- Step 6: Start HTTP server --- */
    printf("\n7. Starting HTTP server...\n");
    http_server_driver_start(&sd_mgr);

    printf("HTTP server ready. Visit http://<pico_ip>/\n");

    /* --- Step 7: Loop --- */
    while (true) {
        sleep_ms(5000);
    }

    return 0;
}
