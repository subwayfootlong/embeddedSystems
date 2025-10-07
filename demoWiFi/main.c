#include <stdio.h>
#include "pico/stdlib.h"
#include "wifi_driver.h"
#include "secrets.h"
#include "lwip/netif.h"
#include "lwip/ip4_addr.h"

int main() {
    stdio_init_all();
    
    // Wait for serial connection
    sleep_ms(2000);
    printf("Starting WiFi connection...\n");

    // Initialise & connect WiFi
    if (wifi_init() != WIFI_OK) return -1;
    if (wifi_connect(WIFI_SSID, WIFI_PASSWORD, 10000) != WIFI_OK) return -1;

    // Wait a moment for IP assignment
    sleep_ms(2000);
    
    // Print IP address
    printf("Pico W IP Address: %s\n", ip4addr_ntoa(netif_ip4_addr(netif_default)));
    
    // Keep the program running
    while (true) {
        sleep_ms(1000);
    }

    wifi_deinit();
    return 0;
}