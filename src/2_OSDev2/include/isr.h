#pragma once
#include <libc/stdint.h>

typedef struct {
    uint32_t ds;                  // Data segment selector
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha
    uint32_t int_no, err_code;    // Interrupt number and error code (if applicable)
    uint32_t eip, cs, eflags, useresp, ss; // Pushed by the processor automatically
} isr_frame_t;

void isr_init(void); // Initializes the ISRs and sets them in the IDT
void isr_handler(isr_frame_t* frame); // Common handler for all ISRs
