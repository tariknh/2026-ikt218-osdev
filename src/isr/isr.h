#ifndef ISR_H
#define ISR_H

#ifdef __cplusplus
extern "C" {
#endif
#include "libc/stdint.h"

//setter opp idt, registrerer isr-funksjonen og konfigurerer cpu-interrupts

void isr_initialize(void);


// IDT Management
// Sett opp en IDT gate - gjør at en ISR/IRQ-vektor peker på en handler
void idt_set_gate(uint8_t vector, void (*handler)(void));

//kalt fra assembly

void isr0(void);
void isr1(void);
void isr14(void);
//funksjonene som håndterer errors

void isr0_handler(void);
void isr1_handler(void);
void isr14_handler(void);

#ifdef __cplusplus
}
#endif


#endif