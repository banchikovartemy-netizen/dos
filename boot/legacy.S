/* PCOS Legacy DOS chainloader.
 * GRUB enters PCOS in 32-bit protected mode. We copy a tiny trampoline below
 * 64 KiB, install our own flat GDT, enter 16-bit protected mode, clear CR0.PE,
 * then use the machine BIOS to read sector 1 of the selected drive to 0000:7C00.
 * This path is intentionally BIOS/CSM-only and never returns to PCOS. */

.section .text
.code32
.global legacy_boot_drive
.type legacy_boot_drive,@function
legacy_boot_drive:
    cli
    mov 4(%esp), %eax
    movb %al, 0x6ff0
    cld
    lea legacy_trampoline_start, %esi
    mov $0x7000, %edi
    mov $(legacy_trampoline_end-legacy_trampoline_start), %ecx
    rep movsb
    lgdt legacy_gdtr
    ljmp $0x18, $0x7000
1:  hlt
    jmp 1b
.size legacy_boot_drive,.-legacy_boot_drive

.align 8
legacy_gdt:
    .quad 0x0000000000000000
    /* selector 08: 32-bit flat code */
    .quad 0x00cf9a000000ffff
    /* selector 10: 32-bit flat data */
    .quad 0x00cf92000000ffff
    /* selector 18: 16-bit code, base 0, limit 64K */
    .quad 0x00009a000000ffff
    /* selector 20: 16-bit data, base 0, limit 64K */
    .quad 0x000092000000ffff
legacy_gdt_end:
legacy_gdtr:
    .word legacy_gdt_end-legacy_gdt-1
    .long legacy_gdt

/* This block is copied byte-for-byte to physical 0x7000. Keep it PIC. */
.code16
legacy_trampoline_start:
    mov $0x20, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss
    mov $0x6e00, %sp

    mov %cr0, %eax
    and $0xfffffffe, %eax
    mov %eax, %cr0
    /* far jump flushes the prefetch queue and establishes real-mode CS */
    .byte 0xea
    .word 0x7000 + (legacy_rm - legacy_trampoline_start)
    .word 0x0000

legacy_rm:
    xor %ax, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss
    mov $0x6e00, %sp

    /* Restore the conventional BIOS PIT rate (~18.2 Hz). */
    mov $0x36, %al
    out %al, $0x43
    xor %al, %al
    out %al, $0x40
    out %al, $0x40

    /* Return the display to BIOS text mode before handing control to DOS. */
    mov $0x0003, %ax
    int $0x10

    mov 0x6ff0, %dl
    xor %ah, %ah
    int $0x13

    xor %ax, %ax
    mov %ax, %es
    mov $0x7c00, %bx
    mov $0x0201, %ax
    xor %cx, %cx
    inc %cl
    xor %dh, %dh
    mov 0x6ff0, %dl
    int $0x13
    jc legacy_fail
    cmpw $0xaa55, 0x7dfe
    jne legacy_fail
    mov 0x6ff0, %dl
    sti
    .byte 0xea
    .word 0x7c00
    .word 0x0000

legacy_fail:
    mov $0x0003, %ax
    int $0x10
    mov $legacy_msg_off, %si
legacy_print:
    lodsb
    test %al,%al
    jz legacy_hang
    mov $0x0007,%bx
    mov $0x0e,%ah
    int $0x10
    jmp legacy_print
legacy_hang:
    cli
    hlt
    jmp legacy_hang

/* runtime offset within copied trampoline, not linked virtual address */
.set legacy_msg_off, 0x7000 + (legacy_msg - legacy_trampoline_start)
legacy_msg:
    .ascii "PCOS LEGACY BOOT ERROR\r\n\0"
legacy_trampoline_end:
.code32
