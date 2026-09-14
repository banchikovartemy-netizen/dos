#ifndef PCOS_ATA_H
#define PCOS_ATA_H
#include "types.h"
void ata_init(void);
int ata_ready(void);
int ata_read_sector(u32 lba, void *buf);
int ata_write_sector(u32 lba, const void *buf);
const char *ata_model(void);
#endif
