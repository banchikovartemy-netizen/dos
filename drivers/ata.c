#include "ata.h"
#include "io.h"
#include "lib.h"
#define ATA_IO 0x1F0
#define ATA_CTL 0x3F6
#define ATA_DATA 0
#define ATA_ERR 1
#define ATA_SECCOUNT 2
#define ATA_LBA0 3
#define ATA_LBA1 4
#define ATA_LBA2 5
#define ATA_HDDEVSEL 6
#define ATA_STATUS 7
#define ATA_COMMAND 7
#define ATA_SR_BSY 0x80
#define ATA_SR_DRQ 0x08
#define ATA_SR_ERR 0x01
static int ok=0;static char model[41]="NONE";
static void wait400(void){inb(ATA_CTL);inb(ATA_CTL);inb(ATA_CTL);inb(ATA_CTL);}
static int wait_not_bsy(void){for(u32 i=0;i<1000000;i++){u8 s=inb(ATA_IO+ATA_STATUS);if(!(s&ATA_SR_BSY))return s;}return -1;}
static int wait_drq(void){for(u32 i=0;i<1000000;i++){u8 s=inb(ATA_IO+ATA_STATUS);if(s&ATA_SR_ERR)return -1;if(!(s&ATA_SR_BSY)&&(s&ATA_SR_DRQ))return s;}return -1;}
void ata_init(void){ok=0;kstrncpy(model,"NONE",sizeof(model));outb(ATA_IO+ATA_HDDEVSEL,0xA0);wait400();outb(ATA_IO+ATA_SECCOUNT,0);outb(ATA_IO+ATA_LBA0,0);outb(ATA_IO+ATA_LBA1,0);outb(ATA_IO+ATA_LBA2,0);outb(ATA_IO+ATA_COMMAND,0xEC);u8 s=inb(ATA_IO+ATA_STATUS);if(!s)return;if(wait_not_bsy()<0)return;if(inb(ATA_IO+ATA_LBA1)||inb(ATA_IO+ATA_LBA2))return;if(wait_drq()<0)return;u16 id[256];for(int i=0;i<256;i++)id[i]=inw(ATA_IO+ATA_DATA);for(int i=0;i<20;i++){model[i*2]=(char)(id[27+i]>>8);model[i*2+1]=(char)id[27+i];}model[40]=0;for(int i=39;i>=0&&model[i]==' ';i--)model[i]=0;ok=1;}
int ata_ready(void){return ok;}
const char*ata_model(void){return model;}
static int select_lba(u32 lba){if(!ok||lba>=0x0FFFFFFFu)return 0;if(wait_not_bsy()<0)return 0;outb(ATA_IO+ATA_HDDEVSEL,(u8)(0xE0|((lba>>24)&0x0F)));wait400();outb(ATA_IO+ATA_SECCOUNT,1);outb(ATA_IO+ATA_LBA0,(u8)lba);outb(ATA_IO+ATA_LBA1,(u8)(lba>>8));outb(ATA_IO+ATA_LBA2,(u8)(lba>>16));return 1;}
int ata_read_sector(u32 lba,void*buf){if(!select_lba(lba))return 0;outb(ATA_IO+ATA_COMMAND,0x20);if(wait_drq()<0)return 0;u16*p=(u16*)buf;for(int i=0;i<256;i++)p[i]=inw(ATA_IO+ATA_DATA);return 1;}
int ata_write_sector(u32 lba,const void*buf){if(!select_lba(lba))return 0;outb(ATA_IO+ATA_COMMAND,0x30);if(wait_drq()<0)return 0;const u16*p=(const u16*)buf;for(int i=0;i<256;i++)outw(ATA_IO+ATA_DATA,p[i]);outb(ATA_IO+ATA_COMMAND,0xE7);return wait_not_bsy()>=0;}
