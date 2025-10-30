#include "http_server_driver.h"
#include "lwip/tcp.h"
#include "ff.h"
#include <stdio.h>
#include <string.h>
#include "pico/time.h"


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
"<!DOCTYPE html><html><head><title>Pico Graph Dashboard</title>"
"<meta name='viewport' content='width=device-width,initial-scale=1'>"
"<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>"
"<style>"
"body{font-family:sans-serif;margin:1em;background:#f5f5f5;}"
"h2{color:#007acc;}canvas{max-width:100%;height:300px;background:#fff;border-radius:8px;margin-bottom:2em;}"
"button{padding:.5em 1em;margin-bottom:1em;}"
"</style></head><body>"
"<h2>Pico Sensor Graphs</h2>"
"<button onclick='refresh()'>Refresh</button>"
"<canvas id='chart1'></canvas>"
"<canvas id='chart2'></canvas>"
"<script>"
"let c1,c2;"
"async function refresh(){"
"const r=await fetch('/data');const t=await r.text();"
"const lines=t.trim().split('\\n').slice(1);"
"const data1=[],labels1=[],data2=[],labels2=[];"
"for(const l of lines){const [ts,topic,val]=l.split(',');if(!ts||!topic||!val)continue;"
"const time=new Date(Number(ts)).toLocaleTimeString();"
"if(topic==='pico1/sensor/data'){labels1.push(time);data1.push(parseFloat(val));}"
"else if(topic==='pico2/sensor/data'){labels2.push(time);data2.push(parseFloat(val));}}"
"function make(id,labels,data,color,title){return new Chart(document.getElementById(id),{type:'line',data:{labels:labels,datasets:[{label:title,data:data,borderColor:color,fill:false,tension:0.1}]},options:{scales:{y:{beginAtZero:true}}}});}"
"if(c1){c1.destroy();c2.destroy();}"
"c1=make('chart1',labels1,data1,'red','Pico 1 - pico1/sensor/data');"
"c2=make('chart2',labels2,data2,'blue','Pico 2 - pico2/sensor/data');"
"}window.onload=refresh;"
"</script></body></html>";






/* ==========================================================
   TCP send-complete callback (ensures graceful close)
   ========================================================== */
static bool close_timer_callback(alarm_id_t id, void *user_data) {
    struct tcp_pcb *tpcb = (struct tcp_pcb *)user_data;
    printf("[timer] closing connection after delay\n");
    tcp_close(tpcb);
    return false; // one-shot
}

static err_t on_sent(void *arg, struct tcp_pcb *tpcb, u16_t len) {
    LWIP_UNUSED_ARG(arg);
    printf("[on_sent] %u bytes acknowledged, scheduling close\n", len);
    add_alarm_in_ms(300, close_timer_callback, tpcb, false);
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

    // Return OK, let on_sent() close once lwIP finishes
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
