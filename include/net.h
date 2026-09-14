#ifndef PCOS_NET_H
#define PCOS_NET_H
#include "types.h"
void net_init(void);
void net_poll(void);
int net_ready(void);
const char *net_driver(void);
void net_get_mac(char out[18]);
void net_get_ip(char out[16]);
void net_get_gateway(char out[16]);
void net_get_dns(char out[16]);
void net_request_dhcp(void);
void net_ping_gateway(void);
u32 net_rx_packets(void);
u32 net_tx_packets(void);
u32 net_ping_replies(void);
const char *net_state(void);
int net_web_open(const char *url);
int net_web_read(char *out,u32 outn);
const char *net_web_state(void);
#endif
