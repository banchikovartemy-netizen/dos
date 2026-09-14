#ifndef PCOS_IDT_H
#define PCOS_IDT_H
#include "types.h"
void idt_init(void);
__attribute__((noreturn)) void fault_handler(u32 vector, u32 error);
#endif
