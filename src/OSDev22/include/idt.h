#pragma once

#include "libc/stdint.h"

/*
 * Interrupt Descriptor Table (IDT) - definerer hvor CPU-en skal hoppe
 * når et interrupt oppstår. Hver entry er 8 bytes og peker til en
 * interrupt handler (ISR).
 *
 * Strukturen er litt rar fordi base-adressen er splittet i to deler
 * (low og high), akkurat som i GDT. Dette er pga bakoverkompatibilitet
 * med eldre x86-prosessorer.
 */
struct idt_entry {
    uint16_t base_low;    /* Nedre 16 bits av handler-adressen  */
    uint16_t selector;    /* Kodesegment-selector, alltid 0x08  */
    uint8_t  zero;        /* Ubrukt, må være 0                  */
    uint8_t  flags;       /* Type og rettigheter for gaten      */
    uint16_t base_high;   /* Øvre 16 bits av handler-adressen   */
} __attribute__((packed));

/*
 * IDT-peker som sendes til lidt-instruksjonen.
 * Samme oppsett som GDT-pekeren - størrelse og adresse.
 */
struct idt_ptr {
    uint16_t limit;   /* Størrelse på IDT i bytes, minus 1 */
    uint32_t base;    /* Startadressen til IDT-tabellen    */
} __attribute__((packed));

/* Vi har plass til 256 ulike interrupts */
#define IDT_ENTRY_COUNT 256

/*
 * Flagg for IDT-entries:
 *   0x8E = 32-bit interrupt gate, ring 0 (kun kernel kan trigge)
 *   0xEE = 32-bit interrupt gate, ring 3 (brukerprogram kan trigge)
 */
#define IDT_FLAG_KERNEL 0x8E
#define IDT_FLAG_USER   0xEE

/* Setter opp én enkelt IDT-entry */
void idt_set_entry(int num, uint32_t base, uint16_t selector, uint8_t flags);

/* Initialiserer og laster IDT-en inn i CPU-en */
void idt_init(void);