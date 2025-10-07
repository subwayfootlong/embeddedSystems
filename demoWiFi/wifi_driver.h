#ifndef WIFI_DRIVER_H
#define WIFI_DRIVER_H

#include "pico/cyw43_arch.h"

// Return codes
#define WIFI_OK     0
#define WIFI_ERROR -1

bool wifi_is_connected(void);

// Initialise WiFi 
int wifi_init(void);

// Connect to SSID with password, timeout in ms
int wifi_connect(const char *ssid, const char *pass, uint32_t timeout_ms);

// Disconnect WiFi
void wifi_deinit(void);

#endif