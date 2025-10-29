#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

// ---- Pico W default lwIP options ----
#define NO_SYS                      1
#define LWIP_SOCKET                 0
#define LWIP_NETCONN                0
#define LWIP_TCP                    1
#define TCP_TTL                     255
#define LWIP_ICMP                   1
#define LWIP_DHCP                   1
#define LWIP_DNS                    1
#define LWIP_HAVE_LOOPIF            0
#define LWIP_NETIF_LOOPBACK         0
#define MEM_ALIGNMENT               4
#define MEM_SIZE                    4000
#define MEMP_NUM_PBUF               10
#define MEMP_NUM_TCP_PCB            5
#define PBUF_POOL_SIZE              8
#define TCP_MSS                     1460
#define TCP_SND_BUF                 (2 * TCP_MSS)
#define TCP_WND                     (2 * TCP_MSS)
#define LWIP_STATS                  0
#define LWIP_PROVIDE_ERRNO          1
#define LWIP_NETIF_STATUS_CALLBACK  1

#endif /* _LWIPOPTS_H */
