/*
Name: irq.h
Project: LocOS
Description: This file contains the definitions and function declarations for handling hardware interrupts (IRQs)
*/

#ifndef IRQ_H//Start include guard
#define IRQ_H  // Define include guard macro

#include <libc/stdint.h>//Integer types
#include <libc/stdbool.h>//Boolean type

void irq_init(void);// Initialize PIC controller
void irq_handler_c(uint32_t irq_no); // Route IRQ to handler
int  kbd_try_getchar(void);   /* returns -1 if none, else unsigned char in low byte */
int  kbd_getchar(void);       /* blocks (hlt loop) until a char is available */
void kbd_set_echo(bool enabled);

#endif                          