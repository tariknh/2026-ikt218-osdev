#ifndef ISR_H
#define ISR_H

#include "libc/stdint.h"

// This matches the stack layout from your assembly stub
typedef struct {
    uint32_t ds;                                     // Data segment
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha
    uint32_t int_no, err_code;                       // Interrupt number and error code
    uint32_t eip, cs, eflags, useresp, ss;           // Pushed by CPU
} __attribute__((packed)) registers_t;

// ISR handler function prototypes
void isr_handler(registers_t *r);
void isr_install();

// These are declared in assembly
extern void isr0();
extern void isr1();
extern void isr2();

// Convenience macros for ISR flags
#define ISR_FLAG_INTERRUPT_GATE 0x8E
#define ISR_FLAG_TRAP_GATE      0x8F

#endif