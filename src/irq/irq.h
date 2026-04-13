#ifndef IRQ_H
#define IRQ_H

#ifdef __cplusplus
extern "C" {
#endif

#include "libc/stdint.h"


// Disse blir remapped til ISR 32-47 for å unngå konflikt med CPU exceptions.
// IRQ Initialization- Initialiser IRQ subsystem: PIC remapping, IDT registration, enable interrupts
void irq_initialize(void);
// Assembly stubs - brukt av IDT for å håndtere IRQs
// Disse assembly-funksjonene lagrer registers, celler C-handler, og returnerer

void irq0(void);   // Timer IRQ
void irq1(void);   // Keyboard IRQ
void irq2(void);   // Cascade (slave PIC)
void irq3(void);   // Serial Port 2
void irq4(void);   // Serial Port 1
void irq5(void);   // Parallel Port 2
void irq6(void);   // Floppy Disk
void irq7(void);   // Parallel Port 1
void irq8(void);   // Real Time Clock
void irq9(void);   // Reserved/Redirection
void irq10(void);  // Reserved
void irq11(void);  // Reserved
void irq12(void);  // PS/2 Mouse
void irq13(void);  // FPU Co-processor
void irq14(void);  // ATA Hard Disk 1
void irq15(void);  // ATA Hard Disk 2


// C-level IRQ Handlers
// kalles fra assembly stubs

void irq0_handler(void);   // Timer
void irq1_handler(void);   // Keyboard
void irq_dispatcher(uint8_t irq);  // Dispatcher for andre IRQs

#ifdef __cplusplus
}
#endif

#endif
