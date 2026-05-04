#ifndef ISR_H
#define ISR_H

// Includes fixed-size integer types
#include "libc/stdint.h"
// CPU register state saved during an interrupt
struct registers {
    uint32_t ds;// Data segment selector
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;     // General-purpose registers
    uint32_t int_no, err_code; // Interrupt number and error code
    uint32_t eip, cs, eflags, useresp, ss;     // CPU state pushed automatically by the processor
};
// Installs CPU exception handlers
void isr_install(void);
// Main ISR dispatcher
void isr_handler(struct registers* regs);

#endif