#include "mqtt_driver.h"
#include "secrets.h"
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"

static mqtt_client_t *mqtt_client = NULL;
static mqtt_status_t mqtt_status = MQTT_STATUS_DISCONNECTED;
static mqtt_message_callback_t user_callback = NULL;

// Buffer to store the current topic between callbacks
static char current_topic[64] = "";

// MQTT connection callback
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        printf("MQTT connected\n");
        mqtt_status = MQTT_STATUS_CONNECTED;
    } else {
        printf("MQTT connection failed (status=%d)\n", status);
        mqtt_status = MQTT_STATUS_ERROR;
    }
}

// MQTT incoming publish callback - store the topic
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
    printf("[DEBUG] Incoming publish - Topic: '%s', Length: %lu\n", topic ? topic : "NULL", (unsigned long)tot_len);
    
    // Store the topic for the data callback
    if (topic) {
        strncpy(current_topic, topic, sizeof(current_topic) - 1);
        current_topic[sizeof(current_topic) - 1] = '\0';
    } else {
        current_topic[0] = '\0';
    }
}

// MQTT incoming data callback - use stored topic
static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    // Copy data to temporary buffer
    char payload[256];
    if (len < sizeof(payload)) {
        memcpy(payload, data, len);
        payload[len] = '\0';
        
        printf("[DEBUG] Data callback - Topic: '%s', Payload: %s\n", current_topic, payload);
        
        if (user_callback && current_topic[0] != '\0') {
            user_callback(current_topic, payload, len);
        }
        
        printf("Received: %s\n", payload);
    }
    
    // Clear topic for next message
    current_topic[0] = '\0';
}

// MQTT subscribe callback
static void mqtt_sub_request_cb(void *arg, err_t result) {
    if (result == ERR_OK) {
        printf("Subscribe successful\n");
    } else {
        printf("Subscribe failed (err=%d)\n", result);
    }
}

// MQTT publish callback
static void mqtt_pub_request_cb(void *arg, err_t result) {
    if (result == ERR_OK) {
        printf("Publish successful\n");
    } else {
        printf("Publish failed (err=%d)\n", result);
    }
}

int mqtt_init(const char* client_id) {
    mqtt_client = mqtt_client_new();
    if (!mqtt_client) {
        printf("Failed to create MQTT client\n");
        return MQTT_ERROR;
    }
    
    mqtt_status = MQTT_STATUS_DISCONNECTED;
    current_topic[0] = '\0';
    printf("MQTT client initialized (ID: %s)\n", client_id);
    return MQTT_OK;
}

int mqtt_connect(const char* broker_ip, uint16_t port, mqtt_message_callback_t callback) {
    if (!mqtt_client) {
        printf("MQTT client not initialized\n");
        return MQTT_ERROR;
    }
    
    user_callback = callback;
    
    // Configure MQTT client info
    struct mqtt_connect_client_info_t ci;
    memset(&ci, 0, sizeof(ci));
    ci.client_id = MQTT_CLIENT_ID;
    ci.keep_alive = 60;
    ci.will_topic = NULL;
    ci.will_msg = NULL;
    
    // Convert IP string to ip_addr_t
    ip_addr_t broker_addr;
    if (!ip4addr_aton(broker_ip, &broker_addr)) {
        printf("Invalid broker IP address\n");
        return MQTT_ERROR;
    }
    
    // Set callbacks
    mqtt_set_inpub_callback(mqtt_client, 
                           mqtt_incoming_publish_cb, 
                           mqtt_incoming_data_cb, 
                           NULL);
    
    // Connect to broker
    mqtt_status = MQTT_STATUS_CONNECTING;
    err_t err = mqtt_client_connect(mqtt_client, 
                                    &broker_addr, 
                                    port, 
                                    mqtt_connection_cb, 
                                    NULL, 
                                    &ci);
    
    if (err != ERR_OK) {
        printf("MQTT connect failed (err=%d)\n", err);
        mqtt_status = MQTT_STATUS_ERROR;
        return MQTT_ERROR;
    }
    
    printf("Connecting to MQTT broker %s:%d...\n", broker_ip, port);
    return MQTT_OK;
}

int mqtt_publish_message(const char* topic, const char* payload, uint8_t qos, uint8_t retain) {
    if (mqtt_status != MQTT_STATUS_CONNECTED) {
        printf("MQTT not connected\n");
        return MQTT_ERROR;
    }
    
    err_t err = mqtt_publish(mqtt_client, 
                            topic, 
                            payload, 
                            strlen(payload), 
                            qos, 
                            retain, 
                            mqtt_pub_request_cb, 
                            NULL);
    
    if (err != ERR_OK) {
        printf("MQTT publish failed (err=%d)\n", err);
        return MQTT_ERROR;
    }
    
    return MQTT_OK;
}

int mqtt_subscribe_topic(const char* topic, uint8_t qos) {
    if (mqtt_status != MQTT_STATUS_CONNECTED) {
        printf("MQTT not connected\n");
        return MQTT_ERROR;
    }
    
    err_t err = mqtt_subscribe(mqtt_client, 
                              topic, 
                              qos, 
                              mqtt_sub_request_cb, 
                              NULL);
    
    if (err != ERR_OK) {
        printf("MQTT subscribe failed (err=%d)\n", err);
        return MQTT_ERROR;
    }
    
    printf("Subscribing to topic: %s\n", topic);
    return MQTT_OK;
}

int mqtt_unsubscribe_topic(const char* topic) {
    if (mqtt_status != MQTT_STATUS_CONNECTED) {
        return MQTT_ERROR;
    }
    
    err_t err = mqtt_unsubscribe(mqtt_client, topic, mqtt_sub_request_cb, NULL);
    return (err == ERR_OK) ? MQTT_OK : MQTT_ERROR;
}

mqtt_status_t mqtt_get_status(void) {
    return mqtt_status;
}

void mqtt_disconnect_client(void) {
    if (mqtt_client) {
        mqtt_disconnect(mqtt_client);
        mqtt_client_free(mqtt_client);
        mqtt_client = NULL;
        mqtt_status = MQTT_STATUS_DISCONNECTED;
        printf("MQTT disconnected\n");
    }
}

void mqtt_poll(void) {
    // This function can be used for any periodic MQTT maintenance
    // Currently lwIP handles polling internally
}