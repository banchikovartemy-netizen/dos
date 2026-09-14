#ifndef PCOS_PCI_H
#define PCOS_PCI_H
#include "types.h"
u32 pci_read32(u8 bus,u8 slot,u8 func,u8 off);
void pci_write32(u8 bus,u8 slot,u8 func,u8 off,u32 value);
int pci_find(u16 vendor,u16 device,u8 *bus,u8 *slot,u8 *func);
#endif
