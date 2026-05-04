#pragma once

#include "libc/stdint.h"
#include "isr.h"

/*
 * IRQ-håndtering for hardware-interrupts.
 *
 * PIC-en (Programmable Interrupt Controller) sender hardware-interrupts
 * til CPU-en. Problemet er at PIC-en som standard mapper IRQ 0-7 til
 * interrupt 8-15, som kræsjer med CPU-exceptions. Derfor må vi
 * remapper PIC-en så IRQ-ene havner på interrupt 32-47 istedenfor.
 *
 * IRQ 0  = Timer (PIT)         -> interrupt 32
 * IRQ 1  = Tastatur (PS/2)     -> interrupt 33
 * IRQ 2  = Kaskade (slave PIC) -> interrupt 34
 * ...osv opp til...
 * IRQ 15 = Sekundær ATA disk   -> interrupt 47
 */

/* IRQ-ene mappes til interrupt 32-47 etter remapping */
#define IRQ_OFFSET 32

/* Funksjonstype for custom IRQ-handlere */
typedef void (*irq_handler_t)(struct isr_frame* frame);

/* Registrer en custom handler for en bestemt IRQ (0-15) */
void irq_register_handler(int irq, irq_handler_t handler);

/* Setter opp PIC-remapping og registrerer IRQ-stubbene i IDT-en */
void irq_init(void);

/* Felles C-handler som alle IRQ-stubbene kaller */
void irq_handler(struct isr_frame* frame);