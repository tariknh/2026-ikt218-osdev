#ifndef UIAOS_KERNEL_IDT_H
#define UIAOS_KERNEL_IDT_H

#include <stdint.h>

typedef struct registers {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} registers_t;

typedef void (*isr_t)(registers_t *regs);

void idt_install(void);
void register_interrupt_handler(uint8_t interrupt_number, isr_t handler);

#endif
