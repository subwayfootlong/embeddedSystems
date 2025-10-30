#include "http_server_driver.h"
#include "lwip/tcp.h"
#include "ff.h"
#include <stdio.h>
#include <string.h>

static SD_Manager *g_sd = NULL;

/* ==========================================================
   Helper: read CSV data from SD card
   ========================================================== */
static const char *read_csv(char *buf, size_t maxlen) {
    if (!g_sd || !g_sd->mounted)
        return "SD card not mounted\n";

    FIL f;
    UINT br;
    if (f_open(&f, "sensor_log.csv", FA_READ) != FR_OK)
        return "File not found\n";

    f_read(&f, buf, maxlen - 1, &br);
    f_close(&f);
    buf[br] = '\0';
    return buf;
}

/* ==========================================================
   Static HTML dashboard page
   ========================================================== */
static const char html_index[] =
"<!DOCTYPE html><html><head><title>Pico Dashboard</title>"
"<meta name='viewport' content='width=device-width,initial-scale=1'>"
"<style>"
"body{font-family:sans-serif;margin:2em;background:#f5f5f5;color:#222;}"
"h2{color:#007acc;}table{border-collapse:collapse;width:100%;margin-bottom:2em;background:#fff;}"
"th,td{border:1px solid #ccc;padding:4px;text-align:left;}th{background:#eee;}"
"button{padding:.5em 1em;margin-bottom:1em;}"
"</style></head><body>"
"<h2>Pico Sensor Dashboard</h2>"
"<button onclick='refresh()'>Refresh</button>"
"<div id='tables'>Loading...</div>"
"<script>"
"async function refresh(){"
"const r=await fetch('/data');const t=await r.text();"
"const lines=t.trim().split('\\n').slice(1);"
"let p1='',p2='';"
"for(const l of lines){const [ts,topic,val]=l.split(',');if(!ts||!topic||!val)continue;"
"const time=new Date(Number(ts)).toLocaleTimeString();"
"if(topic==='pico1/sensor/data')p1+=`<tr><td>${time}</td><td>${val}</td></tr>`;"
"else if(topic==='pico2/sensor/data')p2+=`<tr><td>${time}</td><td>${val}</td></tr>`;}"
"function table(title,rows){return `<h3>${title}</h3><table><tr><th>Time</th><th>Value</th></tr>${rows}</table>`;}"
"document.getElementById('tables').innerHTML="
"table('Pico 1 (pico1/sensor/data)',p1)+table('Pico 2 (pico2/sensor/data)',p2);"
"}window.onload=refresh;"
"</script></body></html>";





/* ==========================================================
   TCP send-complete callback (ensures graceful close)
   ========================================================== */
static err_t on_sent(void *arg, struct tcp_pcb *tpcb, u16_t len) {
    LWIP_UNUSED_ARG(arg);
    LWIP_UNUSED_ARG(len);
    tcp_close(tpcb);
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
    static char buffer[2048];
    char header[128];

    if (strncmp(req, "GET /data", 9) == 0) {
        const char *content = read_csv(buffer, sizeof(buffer));
        snprintf(header, sizeof(header),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: text/plain\r\n"
                 "Content-Length: %u\r\n"
                 "Connection: close\r\n\r\n",
                 (unsigned)strlen(content));
        tcp_write(tpcb, header, strlen(header), TCP_WRITE_FLAG_COPY);
        tcp_write(tpcb, content, strlen(content), TCP_WRITE_FLAG_COPY);
    } else {
        const char *page = html_index;
        snprintf(header, sizeof(header),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: text/html\r\n"
                 "Content-Length: %u\r\n"
                 "Connection: close\r\n\r\n",
                 (unsigned)strlen(page));
        tcp_write(tpcb, header, strlen(header), TCP_WRITE_FLAG_COPY);
        tcp_write(tpcb, page, strlen(page), TCP_WRITE_FLAG_COPY);
    }

    tcp_output(tpcb);
    pbuf_free(p);

    // Wait until data is sent before closing
    tcp_sent(tpcb, on_sent);
    return ERR_OK;
}

/* ==========================================================
   Accept callback (new connection)
   ========================================================== */
static err_t accept_cb(void *arg, struct tcp_pcb *newpcb, err_t err) {
    LWIP_UNUSED_ARG(arg);
    LWIP_UNUSED_ARG(err);
    tcp_recv(newpcb, recv_cb);
    return ERR_OK;
}

/* ==========================================================
   Public API: Start/Stop HTTP server
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
