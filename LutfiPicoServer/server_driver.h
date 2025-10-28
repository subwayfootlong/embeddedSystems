#ifndef SERVER_DRIVER_H
#define SERVER_DRIVER_H

#include <stdint.h>

// Server initialization function
int server_init(void);

// Integrated message handler for server
void server_message_handler(const char* topic, const char* payload, uint16_t payload_len);

#endif // SERVER_DRIVER_H