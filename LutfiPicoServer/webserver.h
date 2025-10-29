#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <stdbool.h>
#include <stdint.h>
#include "lwip/err.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"

err_t serve_json(struct tcp_pcb *pcb);
err_t serve_html(struct tcp_pcb *pcb);
err_t on_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);
err_t on_accept(void *arg, struct tcp_pcb *pcb, err_t err);

err_t webserver_start(uint16_t port);
void webserver_stop(void);
void webserver_set_acd_reading(uint32_t ppm, bool valid);

#endif
