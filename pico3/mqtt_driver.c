#include "mqtt_driver.h"
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"

// ==========================
// Global & Static Variables
// ==========================
static mqtt_client_t *mqtt_client = NULL;
static mqtt_status_t mqtt_status = MQTT_STATUS_DISCONNECTED;
static mqtt_message_callback_t user_callback = NULL;

// 🔧 NEW: store the most recent topic between callbacks
static char last_topic[128] = {0};

// ==========================
// Connection Callback
// ==========================
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        printf("MQTT connected\n");
        mqtt_status = MQTT_STATUS_CONNECTED;
    } else {
        printf("MQTT connection failed (status=%d)\n", status);
        mqtt_status = MQTT_STATUS_ERROR;
    }
}

// ==========================
// Incoming Publish Callback
// ==========================
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
    // Save the topic for the data callback
    strncpy(last_topic, topic, sizeof(last_topic) - 1);
    last_topic[sizeof(last_topic) - 1] = '\0';

    // Optional: only print if verbose debugging is needed
    // printf("Incoming publish on topic: %s (length: %lu)\n", topic, (unsigned long)tot_len);
}

// ==========================
// Incoming Data Callback
// ==========================
static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    // Copy data safely to a null-terminated buffer
    char payload[256];
    if (len >= sizeof(payload))
        len = sizeof(payload) - 1;

    memcpy(payload, data, len);
    payload[len] = '\0';

    // Optional: only print if verbose debugging is needed
    // printf("Received message on topic: %s (payload: %s)\n", last_topic, payload);

    // 🔧 Use the stored topic name
    if (user_callback) {
        user_callback(last_topic, payload, len);
    }

    // Clear the topic if this was the last packet in the message
    if (flags & MQTT_DATA_FLAG_LAST) {
        memset(last_topic, 0, sizeof(last_topic));
    }
}

// ==========================
// Subscribe Callback
// ==========================
static void mqtt_sub_request_cb(void *arg, err_t result) {
    const char* topic = (const char*)arg;
    if (result == ERR_OK) {
        printf("Subscribe to %s successful\n", topic);
    } else {
        printf("Subscribe to %s failed (err=%d)\n", topic, result);
    }
}

// ==========================
// Publish Callback
// ==========================
static void mqtt_pub_request_cb(void *arg, err_t result) {
    const char* topic = (const char*)arg;
    if (result == ERR_OK) {
        printf("Publish to %s successful\n", topic);
    } else {
        printf("Publish to %s failed (err=%d)\n", topic, result);
    }
}

// ==========================
// Client Initialization
// ==========================
int mqtt_init(const char* client_id) {
    mqtt_client = mqtt_client_new();
    if (!mqtt_client) {
        printf("Failed to create MQTT client\n");
        return MQTT_ERROR;
    }

    mqtt_status = MQTT_STATUS_DISCONNECTED;
    printf("MQTT client initialized (ID: %s)\n", client_id);
    return MQTT_OK;
}

// ==========================
// Connect to Broker
// ==========================
int mqtt_connect(const char* broker_ip, uint16_t port, mqtt_message_callback_t callback) {
    if (!mqtt_client) {
        printf("MQTT client not initialized\n");
        return MQTT_ERROR;
    }

    user_callback = callback;

    struct mqtt_connect_client_info_t ci;
    memset(&ci, 0, sizeof(ci));
    ci.client_id = "pico_w_client";
    ci.keep_alive = 60;
    ci.will_topic = NULL;
    ci.will_msg = NULL;

    ip_addr_t broker_addr;
    if (!ip4addr_aton(broker_ip, &broker_addr)) {
        printf("Invalid broker IP address\n");
        return MQTT_ERROR;
    }

    // Set the combined incoming publish/data callbacks
    mqtt_set_inpub_callback(mqtt_client,
                            mqtt_incoming_publish_cb,
                            mqtt_incoming_data_cb,
                            NULL);

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

// ==========================
// Publish Message
// ==========================
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
                             (void*)topic);

    if (err != ERR_OK) {
        printf("MQTT publish failed (err=%d)\n", err);
        return MQTT_ERROR;
    }

    return MQTT_OK;
}

// ==========================
// Subscribe / Unsubscribe
// ==========================
int mqtt_subscribe_topic(const char* topic, uint8_t qos) {
    if (mqtt_status != MQTT_STATUS_CONNECTED) {
        printf("MQTT not connected\n");
        return MQTT_ERROR;
    }

    err_t err = mqtt_subscribe(mqtt_client, topic, qos, mqtt_sub_request_cb, (void*)topic);
    if (err != ERR_OK) {
        printf("MQTT subscribe failed (err=%d)\n", err);
        return MQTT_ERROR;
    }

    return MQTT_OK;
}

int mqtt_unsubscribe_topic(const char* topic) {
    if (mqtt_status != MQTT_STATUS_CONNECTED) {
        return MQTT_ERROR;
    }

    err_t err = mqtt_unsubscribe(mqtt_client, topic, mqtt_sub_request_cb, (void*)topic);
    return (err == ERR_OK) ? MQTT_OK : MQTT_ERROR;
}

// ==========================
// Status / Disconnect / Poll
// ==========================
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
    // lwIP handles polling automatically, so this is optional.
}

// Wait for MQTT connection with timeout
bool mqtt_wait_connection(uint32_t timeout_ms) {
    uint32_t start_time = to_ms_since_boot(get_absolute_time());
    
    while (mqtt_get_status() == MQTT_STATUS_CONNECTING) {
        if (to_ms_since_boot(get_absolute_time()) - start_time > timeout_ms) {
            printf("MQTT connection timeout\n");
            return false;
        }
        sleep_ms(100);
    }
    
    return (mqtt_get_status() == MQTT_STATUS_CONNECTED);
}