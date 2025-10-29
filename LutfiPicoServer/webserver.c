#include "webserver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "index_html.h"
#include "lwip/ip4_addr.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"

static struct tcp_pcb *server_pcb = NULL;
static uint32_t latest_acd_ppm = 0;
static bool latest_acd_valid = false;

// ---- serve JSON data ----
err_t serve_json(struct tcp_pcb *pcb) {
    char json[256];
    static int t = 0;

    // Simulated sensor values (replace with real sensors later)
    float temp = 20.0f + (rand() % 100) / 10.0f;
    float humidity = 40.0f + (rand() % 200) / 10.0f;
    float light = (rand() % 1000);
    float acd = latest_acd_valid ? (float)latest_acd_ppm : -1.0f;

    sprintf(json,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Connection: close\r\n"
        "\r\n"
        "{\"time\":%d,\"temp\":%.2f,\"humidity\":%.2f,\"light\":%.0f,\"acd\":%.0f}",
        t++, temp, humidity, light, acd);

    tcp_write(pcb, json, strlen(json), TCP_WRITE_FLAG_COPY);
    tcp_output(pcb);
    sleep_ms(5);
    tcp_close(pcb);
    return ERR_OK;
}

// ---- serve HTML page ----
err_t serve_html(struct tcp_pcb *pcb) {
    const char *header =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n"
        "\r\n";
    tcp_write(pcb, header, strlen(header), TCP_WRITE_FLAG_COPY);
    tcp_write(pcb, index_html, index_html_len, TCP_WRITE_FLAG_COPY);
    tcp_output(pcb);
    sleep_ms(5);
    tcp_close(pcb);
    return ERR_OK;
}

// ---- handle incoming data ----
err_t on_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
    if (!p) {
        tcp_close(pcb);
        return ERR_OK;
    }

    // Copy HTTP request
    char buf[256];
    int len = p->len;
    if (len > sizeof(buf) - 1) len = sizeof(buf) - 1;
    memcpy(buf, p->payload, len);
    buf[len] = '\0';
    tcp_recved(pcb, p->tot_len);
    pbuf_free(p);

    if (strstr(buf, "GET /data") != NULL)
        serve_json(pcb);
    else
        serve_html(pcb);

    return ERR_OK;
}

// ---- when new connection accepted ----
err_t on_accept(void *arg, struct tcp_pcb *pcb, err_t err) {
    tcp_recv(pcb, on_recv);
    return ERR_OK;
}

err_t webserver_start(uint16_t port) {
    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb) {
        return ERR_MEM;
    }

    err_t err = tcp_bind(pcb, IP_ANY_TYPE, port);
    if (err != ERR_OK) {
        tcp_close(pcb);
        return err;
    }

    pcb = tcp_listen(pcb);
    tcp_accept(pcb, on_accept);
    server_pcb = pcb;

    printf("Webserver ready! Open http://%s/\n",
           ip4addr_ntoa(netif_ip4_addr(netif_list)));
    return ERR_OK;
}

void webserver_stop(void) {
    if (!server_pcb) {
        return;
    }

    tcp_close(server_pcb);
    server_pcb = NULL;
}

void webserver_set_acd_reading(uint32_t ppm, bool valid) {
    latest_acd_valid = valid;
    if (valid) {
        latest_acd_ppm = ppm;
    }
}
