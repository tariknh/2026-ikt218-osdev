#ifndef ISR_H
#define ISR_H

#ifdef __cplusplus
extern "C" {
#endif
#include "libc/stdint.h"

// Setter opp IDT og registrerer ISR-rutinene vi bruker.
void isr_initialize(void);

// Kobler en vektor til riktig handler i IDT.
void idt_set_gate(uint8_t vector, void (*handler)(void));

void isr0(void);
void isr1(void);
void isr14(void);

void isr0_handler(void);
void isr1_handler(void);
void isr14_handler(void);

#ifdef __cplusplus
}
#endif


#endif