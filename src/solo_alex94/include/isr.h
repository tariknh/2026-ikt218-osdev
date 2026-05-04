#ifndef ISR_H
#define ISR_H

#include <libc/stdint.h>

struct registers {
    uint32_t ds;
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t useless_esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t int_no;
    uint32_t err_code;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
};

void isr0_handler(void);
void isr1_handler(void);
void isr2_handler(void);
void isr_handler(struct registers* regs);

#endif
