#pragma once

#include "libc/stdint.h"

/*
 * Denne strukturen representerer tilstanden til CPU-en
 * når et interrupt skjer. Den fylles av assembly-stubbene
 * som pusher alt på stacken før de kaller C-handleren.
 *
 * Rekkefølgen her MÅ matche rekkefølgen vi pusher i asm.
 */
struct isr_frame {
    /* Registre pushet av vår assembly-kode (pusha) */
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;

    /* Pushet av vår stub: interrupt-nummer og evt. error code */
    uint32_t int_no;
    uint32_t err_code;

    /* Pushet automatisk av CPU-en når interruptet skjer */
    uint32_t eip, cs, eflags, useresp, ss;
};

/*
 * Registrerer alle 32 CPU-exceptions (ISR 0-31) i IDT-en.
 * Må kalles etter idt_init().
 */
void isr_init(void);

/*
 * Felles C-handler som alle ISR-stubbene kaller.
 * Skriver ut hvilken exception som skjedde.
 */
void isr_handler(struct isr_frame* frame);