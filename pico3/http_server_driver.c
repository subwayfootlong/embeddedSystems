#include "http_server_driver.h"
#include "lwip/tcp.h"
#include "ff.h"
#include <stdio.h>
#include <string.h>
#include "pico/time.h"

static SD_Manager *g_sd = NULL;
static char buffer[8192];   // shared buffer for HTML or CSV

/* ==========================================================
   Helper: read CSV data from SD card (keep last 20 lines)
   ========================================================== */
static const char *read_csv(char *buf, size_t maxlen) {
    if (!g_sd || !g_sd->mounted)
        return "SD card not mounted\n";

    FIL f; UINT br;
    if (f_open(&f, "sensor_log.csv", FA_READ) != FR_OK)
        return "File not found\n";

    FSIZE_t sz = f_size(&f);
    size_t start = (sz > maxlen) ? sz - maxlen : 0;
    f_lseek(&f, start);

    f_read(&f, buf, maxlen - 1, &br);
    f_close(&f);
    buf[br] = '\0';

    // skip partial first line if we started mid-file
    char *p = strchr(buf, '\n');
    if (p) memmove(buf, p + 1, strlen(p + 1) + 1);

    // keep only last 20 lines
    int lines = 0;
    for (char *q = buf; *q; q++) if (*q == '\n') lines++;
    if (lines > 20) {
        int skip = lines - 20;
        p = buf;
        while (skip && *p) if (*p++ == '\n') skip--;
        memmove(buf, p, strlen(p) + 1);
    }

    return buf;
}

/* ==========================================================
   Helper: read HTML page from SD card
   ========================================================== */
static const char *read_html(char *buf, size_t maxlen) {
    if (!g_sd || !g_sd->mounted)
        return "SD card not mounted\n";

    FIL f; UINT br;
    FRESULT res = f_open(&f, "index.html", FA_READ);
    if (res != FR_OK) {
        printf("[HTML] open failed: %d\n", res);
        return "File not found\n";
    }

    f_read(&f, buf, maxlen - 1, &br);
    f_close(&f);
    buf[br] = '\0';
    printf("[HTML] read %u bytes from index.html\n", br);
    return buf;
}

/* ==========================================================
   TCP send-complete callback
   ========================================================== */
static err_t on_sent(void *arg, struct tcp_pcb *tpcb, u16_t len) {
    LWIP_UNUSED_ARG(arg);
    printf("[on_sent] %u bytes acknowledged\n", len);
    if (tcp_sndbuf(tpcb) == TCP_SND_BUF) {
        printf("[on_sent] all data sent, closing now\n");
        tcp_close(tpcb);
    }
    return ERR_OK;
}

/* ==========================================================
   TCP receive callback
   ========================================================== */
static err_t recv_cb(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    LWIP_UNUSED_ARG(arg);
    if (!p) {
        tcp_close(tpcb);
        return ERR_OK;
    }

    char *req = (char *)p->payload;
    char header[160];
    const char *body;
    char content_type[32];

    if (strncmp(req, "GET /data", 9) == 0) {
        body = read_csv(buffer, sizeof(buffer));
        strcpy(content_type, "text/plain");
    } else {
        body = read_html(buffer, sizeof(buffer));
        strcpy(content_type, "text/html");
    }

    snprintf(header, sizeof(header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %u\r\n"
             "Connection: close\r\n\r\n",
             content_type, (unsigned)strlen(body));

    tcp_write(tpcb, header, strlen(header), TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb);

    // stream in safe chunks
    size_t total = strlen(body), sent = 0;
    while (sent < total) {
        u16_t chunk = (total - sent > 1024) ? 1024 : (u16_t)(total - sent);
        err_t res = tcp_write(tpcb, body + sent, chunk, TCP_WRITE_FLAG_COPY);
        if (res != ERR_OK) break;
        sent += chunk;
    }
    tcp_output(tpcb);

    printf("[recv_cb] queued %zu bytes at %u ms\n",
           total, (unsigned)to_ms_since_boot(get_absolute_time()));

    pbuf_free(p);
    tcp_sent(tpcb, on_sent);
    return ERR_OK;
}

/* ==========================================================
   Accept callback
   ========================================================== */
static err_t accept_cb(void *arg, struct tcp_pcb *newpcb, err_t err) {
    LWIP_UNUSED_ARG(arg);
    LWIP_UNUSED_ARG(err);
    tcp_recv(newpcb, recv_cb);
    return ERR_OK;
}

/* ==========================================================
   Public API
   ========================================================== */
void http_server_driver_start(SD_Manager *sd_ref) {
    g_sd = sd_ref;

    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb) {
        printf("Failed to create TCP PCB\n");
        return;
    }

    if (tcp_bind(pcb, IP_ANY_TYPE, 80) != ERR_OK) {
        printf("Failed to bind TCP port 80\n");
        tcp_close(pcb);
        return;
    }

    pcb = tcp_listen(pcb);
    tcp_accept(pcb, accept_cb);
    printf("HTTP server running. Access http://<pico_ip>/\n");
}

void http_server_driver_stop(void) {
    g_sd = NULL;
}
