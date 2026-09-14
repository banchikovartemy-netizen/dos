.set ALIGN,    1<<0
.set MEMINFO,  1<<1
.set FLAGS,    ALIGN | MEMINFO
.set MAGIC,    0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)

.section .multiboot,"a"
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

.section .bss
.align 16
stack_bottom:
.skip 16384
stack_top:

.section .text
.global _start
.type _start, @function
.extern kernel_main
_start:
    /* Do not trust firmware/bootloader CPU flags.  PCOS polls hardware and
       installs its own IDT before it ever enables interrupts. */
    cli
    cld

    /* Give C a clean ABI-compatible stack.  GCC on newer Arch releases is
       stricter about i386 stack alignment than older distro compilers. */
    mov $stack_top, %esp
    and $-16, %esp
    sub $8, %esp
    push %ebx              /* mbi_addr */
    push %eax              /* multiboot magic */
    call kernel_main

1:
    cli
    hlt
    jmp 1b
.size _start, . - _start
