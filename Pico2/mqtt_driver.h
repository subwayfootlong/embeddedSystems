#ifndef MQTT_DRIVER_H
#define MQTT_DRIVER_H

#include "lwip/apps/mqtt.h"
#include <stdint.h>

// Return codes
#define MQTT_OK     0
#define MQTT_ERROR -1

// MQTT connection status
typedef enum {
    MQTT_STATUS_DISCONNECTED,
    MQTT_STATUS_CONNECTING,
    MQTT_STATUS_CONNECTED,
    MQTT_STATUS_ERROR
} mqtt_status_t;

// Callback type for incoming messages
typedef void (*mqtt_message_callback_t)(const char* topic, const char* payload, uint16_t payload_len);

// Initialize MQTT client
int mqtt_init(const char* client_id);

// Connect to MQTT broker
int mqtt_connect(const char* broker_ip, uint16_t port, mqtt_message_callback_t callback);

// Publish message (renamed to avoid conflict)
int mqtt_publish_message(const char* topic, const char* payload, uint8_t qos, uint8_t retain);

// Subscribe to topic (renamed to avoid conflict)
int mqtt_subscribe_topic(const char* topic, uint8_t qos);

// Unsubscribe from topic (renamed to avoid conflict)
int mqtt_unsubscribe_topic(const char* topic);

// Check connection status
mqtt_status_t mqtt_get_status(void);

// Disconnect from broker (renamed to avoid conflict)
void mqtt_disconnect_client(void);

// Poll for MQTT events (call regularly in main loop)
void mqtt_poll(void);

// Setup MQTT and connect to broker
void setup_mqtt(void);

#endif
