#ifndef HTTP_SERVER_DRIVER_H
#define HTTP_SERVER_DRIVER_H

#include "sd_driver.h"

// Start HTTP server after WiFi + MQTT + SD initialization
void http_server_driver_start(SD_Manager *sd_ref);

// Optional stop function (not used for Pico)
void http_server_driver_stop(void);

#endif
