#pragma once

#include "libc/stdint.h"


struct registers {
    uint32_t ds;                                    /* saved data segment       */
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; /* pushed by pusha        */
    uint32_t int_no, err_code;                      /* pushed by stub / CPU     */
    uint32_t eip, cs, eflags, useresp, ss;          /* pushed by CPU on entry   */
};


void isr_handler(struct registers *regs);


void irq_handler(struct registers *regs);

uint32_t get_irq_count(uint8_t irq);

/* Returns the last printable ASCII character received from the keyboard. */
char get_last_keypress(void);

/* Block until a key is pressed and return its ASCII character. */
char kb_getchar(void);

/* Peek at the next buffered character without consuming it; 0 if none. */
char kb_peek(void);

/* Remove and return the next buffered character; 0 if none. */
char kb_consume(void);

/* Discard all pending keyboard input. */
void kb_flush(void);
