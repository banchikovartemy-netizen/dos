#ifndef PCOS_LEGACY_H
#define PCOS_LEGACY_H
#include "types.h"
/* Leaves PCOS permanently and chainloads a BIOS drive in 16-bit real mode.
 * 0x80 = first hard disk, 0x81 = second hard disk, 0x00 = floppy A. */
void legacy_boot_drive(u8 bios_drive) __attribute__((noreturn));
#endif
