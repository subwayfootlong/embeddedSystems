#ifndef TIMESTAMP_DRIVER_H
#define TIMESTAMP_DRIVER_H

#include <stdbool.h>
#include <stdint.h>

// Timestamp synchronization function prototypes
bool timestamp_init(const char* request_topic, const char* reply_topic);
bool timestamp_request_sync(void);
bool timestamp_wait_sync(uint32_t timeout_ms);
bool timestamp_is_synchronized(void);
uint64_t timestamp_get_synced_time(void);
void timestamp_reset_sync(void);

// MQTT message callback for timestamp synchronization
void timestamp_mqtt_handler(const char* topic, const char* payload, uint16_t payload_len);

#endif // TIMESTAMP_DRIVER_H