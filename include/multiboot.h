#ifndef PCOS_MULTIBOOT_H
#define PCOS_MULTIBOOT_H
#include "types.h"

#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002u
#define MBI_FLAG_MEM  (1u<<0)
#define MBI_FLAG_CMDLINE (1u<<2)
#define MBI_FLAG_MODS (1u<<3)
#define MBI_FLAG_FB   (1u<<12)

typedef struct __attribute__((packed)) {
    u32 mod_start;
    u32 mod_end;
    u32 string;
    u32 reserved;
} MultibootModule;

typedef struct __attribute__((packed)) {
    u32 flags;
    u32 mem_lower, mem_upper;
    u32 boot_device, cmdline;
    u32 mods_count, mods_addr;
    u32 syms[4];
    u32 mmap_length, mmap_addr;
    u32 drives_length, drives_addr;
    u32 config_table;
    u32 boot_loader_name;
    u32 apm_table;
    u32 vbe_control_info;
    u32 vbe_mode_info;
    u16 vbe_mode;
    u16 vbe_interface_seg;
    u16 vbe_interface_off;
    u16 vbe_interface_len;
    u64 framebuffer_addr;
    u32 framebuffer_pitch;
    u32 framebuffer_width;
    u32 framebuffer_height;
    u8  framebuffer_bpp;
    u8  framebuffer_type;
    u8  framebuffer_red_field_position;
    u8  framebuffer_red_mask_size;
    u8  framebuffer_green_field_position;
    u8  framebuffer_green_mask_size;
    u8  framebuffer_blue_field_position;
    u8  framebuffer_blue_mask_size;
} MultibootInfo;

#endif
