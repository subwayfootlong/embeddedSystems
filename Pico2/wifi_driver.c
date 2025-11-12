#include "wifi_driver.h"
#include "secrets.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "lwip/netif.h"
#include "lwip/ip4_addr.h"

bool wifi_is_connected(void) {
    return cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA) == CYW43_LINK_UP;
}

int wifi_init(void) {
    if (cyw43_arch_init_with_country(CYW43_COUNTRY_SINGAPORE)) {
        printf("Wi-Fi init failed\n");
        return WIFI_ERROR;
    }
    cyw43_arch_enable_sta_mode();
    printf("Wi-Fi init OK\n");
    return WIFI_OK;
}

int wifi_connect(const char *ssid, const char *pass, uint32_t timeout_ms) {
    const int MAX_RETRIES = 5;
    for (int i = 1; i <= MAX_RETRIES; i++) {
        int ret = cyw43_arch_wifi_connect_timeout_ms(
            ssid, pass, CYW43_AUTH_WPA2_AES_PSK, timeout_ms
        );
        if (ret == 0) {
            printf("Connected to Wi-Fi on attempt %d\n", i);
            return WIFI_OK;
        }
        printf("Wi-Fi connect failed (attempt %d/%d, code=%d)\n", i, MAX_RETRIES, ret);
        sleep_ms(2000);
    }
    printf("Wi-Fi connection failed after %d retries\n", MAX_RETRIES);
    return WIFI_ERROR;
}


void wifi_deinit(void) {
    cyw43_arch_deinit();
    printf("Wi-Fi deinitialised\n");
}

void setup_wifi(void) {
    printf("\n1. Connecting to WiFi...\n");
    if (wifi_init() != WIFI_OK) {
        printf("WiFi init failed\n");
        return;
    }
    if (wifi_connect(WIFI_SSID, WIFI_PASSWORD, 10000) != WIFI_OK) {
        printf("WiFi connect failed\n");
        return;
    }

    // Wait for IP assignment
    sleep_ms(2000);
    printf("Pico W IP Address: %s\n", ip4addr_ntoa(netif_ip4_addr(netif_default)));
}
