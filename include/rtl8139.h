#ifndef PCOS_RTL8139_H
#define PCOS_RTL8139_H
#include "types.h"
int rtl8139_init(void);
int rtl8139_ready(void);
void rtl8139_mac(u8 out[6]);
int rtl8139_send(const u8 *data,u32 len);
int rtl8139_recv(u8 *out,u32 max);
#endif
