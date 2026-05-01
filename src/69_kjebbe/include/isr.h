#ifndef ISR_H
#define ISR_H

#include "libc/stdint.h"

// Snapshot of all registers at the moment an interrupt fires.
// The layout must match exactly what isr_common_stub pushes onto the stack.
typedef struct {
  uint32_t ds; // The currently used datasegment
  uint32_t edi, esi, ebp, esp, ebx, edx, ecx,
      eax; // the following variables are pushed onto the stack by the `pusha`
           // asm instruction
  uint32_t int_no, err_code; // these variabels are pushed before our ISR stub
                             // in the isr.asm file
  uint32_t eip, cs, eflags, useresp,
      ss; // CPU automatically pushes these variables upon interrupt
} registers_t;

// Inserts ISR entries into the IDT
void isr_init(void);

// Called by isr_common_stub. Currently only prints interrupt number.
void isr_handler(registers_t *regs);

// Makes entrypoints to various interrupt service routines defined in the
// isr.asm file available in C.
extern void isr0(void); // Division By Zero
extern void isr1(void); // Debug
extern void isr2(void); // Non-Maskable Interrupt

#endif
