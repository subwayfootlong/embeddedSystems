#include "wifi_driver.h"
#include "secrets.h"
#include <stdio.h>

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
    int retCode = cyw43_arch_wifi_connect_timeout_ms(
        ssid, pass, CYW43_AUTH_WPA2_AES_PSK, timeout_ms
    );

    if (retCode) {
        printf("Wi-Fi connect failed (code=%d)\n", retCode);
        return WIFI_ERROR;
    }
    printf("Connected to Wi-Fi: %s\n", ssid);
    return WIFI_OK;
}

void wifi_deinit(void) {
    cyw43_arch_deinit();
    printf("Wi-Fi deinitialised\n");
}