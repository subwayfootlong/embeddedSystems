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
"<!DOCTYPE html><html><head><title>Pico Logger</title>"
"<meta name='viewport' content='width=device-width,initial-scale=1'>"
"<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>"
"<style>"
"body{font-family:sans-serif;margin:2em;background:#f5f5f5;}"
"canvas{max-width:100%;height:400px;background:#fff;border-radius:8px;}"
"button{padding:.5em 1em;margin-top:1em;}"
"</style>"
"</head><body>"
"<h2>Pico Data Logger Dashboard</h2>"
"<canvas id='chart'></canvas>"
"<button onclick='refresh()'>Refresh</button>"
"<script>"
"let chart;"
"async function refresh(){"
"  const r=await fetch('/data');"
"  const text=await r.text();"
"  const lines=text.trim().split('\\n').slice(1);" // skip header
"  const labels=[], values=[];"
"  for(const line of lines){"
"    const [ts,val]=line.split(',');"
"    if(ts && val){"
"      const date=new Date(Number(ts));"
"      labels.push(date.toLocaleTimeString());"
"      values.push(parseFloat(val));"
"    }"
"  }"
"  const data={labels:labels,datasets:[{label:'Sensor Data',data:values,fill:false,borderColor:'blue',tension:0.1}]};"
"  if(chart){chart.data=data;chart.update();}"
"  else{chart=new Chart(document.getElementById('chart'),{type:'line',data:data,options:{scales:{y:{beginAtZero:true}}}});}"
"}"
"window.onload=refresh;"
"</script>"
"</body></html>";


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
