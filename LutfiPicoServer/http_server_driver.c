#include "http_server_driver.h"
#include "lwip/tcp.h"
#include "ff.h"
#include <stdio.h>
#include <string.h>
#include "pico/time.h"

static SD_Manager *g_sd = NULL;
static char buffer[1024];

/* ==========================================================
   Helper: read CSV data from SD card
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

    // skip partial first line
    char *p = strchr(buf, '\n');
    if (p) memmove(buf, p + 1, strlen(p + 1) + 1);

    // keep only last 10 lines
    int lines = 0;
    for (char *q = buf; *q; q++) if (*q == '\n') lines++;
    int skip = (lines > 10) ? lines - 10 : 0;
    p = buf;
    while (skip && *p) if (*p++ == '\n') skip--;
    if (skip == 0 && p > buf) memmove(buf, p, strlen(p) + 1);

    return buf;
}


/* ==========================================================
   Static HTML dashboard page
   ========================================================== */
static const char html_index[]=
"<!doctype html><html><head><meta name=viewport content='width=device-width,initial-scale=1'>"
"<script src=https://cdn.jsdelivr.net/npm/chart.js></script>"
"<style>body{font-family:sans-serif;background:#f5f5f5;margin:1em}"
"h2{color:#07c}canvas{width:100%;height:300px;background:#fff;border-radius:8px}button{padding:.4em 1em}</style>"
"</head><body><h2>Pico Gas Data</h2><button onclick=r()>Refresh</button><canvas id=c></canvas>"
"<script>"
"let g;"
"async function r(){"
"const R=await fetch('/data');"
"const T=(await R.text()).trim().split('\\n').slice(1);"
"const L=[],a=[],b=[],c=[],d=[];"
"for(const l of T){const p=l.split(','),t=p[0],x=p[1];if(!t||!x)continue;"
"const s=new Date(+t).toLocaleTimeString();"
"if(x==='pico1/sensor/data'){L.push(s);a.push(+p[2]);b.push(+p[3]);c.push(+p[4]);}"
"else if(x==='pico2/sensor/data'){d.push(+p[2]);}}"
"if(g)g.destroy();"
"g=new Chart(document.getElementById('c'),{type:'line',data:{labels:L,datasets:["
"{label:'LPG',data:a,borderColor:'red',fill:!1,tension:.1},"
"{label:'CO',data:b,borderColor:'green',fill:!1,tension:.1},"
"{label:'NH3',data:c,borderColor:'orange',fill:!1,tension:.1},"
"{label:'CO2',data:d,borderColor:'blue',fill:!1,tension:.1}"
"]},options:{scales:{y:{beginAtZero:!0}}}});}"
"r();"
"</script></body></html>";

/* ==========================================================
   TCP send-complete callback (ensures graceful close)
   ========================================================== */

static err_t on_sent(void *arg, struct tcp_pcb *tpcb, u16_t len) {
    LWIP_UNUSED_ARG(arg);
    printf("[on_sent] %u bytes acknowledged, closing now\n", len);
    tcp_close(tpcb);   // close immediately
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

    // Determine which page to serve
    if (strncmp(req, "GET /data", 9) == 0) {
        body = read_csv(buffer, sizeof(buffer));
        strcpy(content_type, "text/plain");
    } else {
        body = html_index;
        strcpy(content_type, "text/html");
    }

    snprintf(header, sizeof(header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %u\r\n"
             "Connection: close\r\n\r\n",
             content_type, (unsigned)strlen(body));

    // Write header + body
    tcp_write(tpcb, header, strlen(header), TCP_WRITE_FLAG_COPY);
    tcp_write(tpcb, body, strlen(body), TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb);
    printf("[recv_cb] queued %u bytes at %u ms\n",
           (unsigned)strlen(body), (unsigned)to_ms_since_boot(get_absolute_time()));

    // Free received data
    pbuf_free(p);

    // Register send callback and do NOT close yet
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
