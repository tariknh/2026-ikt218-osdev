/*
Name: irq.h
Project: LocOS
Description: This file contains the definitions and function declarations for handling hardware interrupts (IRQs)
*/

#ifndef IRQ_H//Start include guard
#define IRQ_H  // Define include guard macro

#include <libc/stdint.h>//Integer types
#include <libc/stdbool.h>//Boolean type

// Arrow key codes used by the games
#define KBD_KEY_UP    0x11
#define KBD_KEY_DOWN  0x12
#define KBD_KEY_LEFT  0x13
#define KBD_KEY_RIGHT 0x14

void irq_init(void);// Initialize PIC controller
void irq_handler_c(uint32_t irq_no); // Route IRQ to handler
// Keyboard input helpers
int  kbd_try_getchar(void);   /* returns -1 if none, else unsigned char in low byte */
int  kbd_getchar(void);       /* blocks (hlt loop) until a char is available */
void kbd_set_echo(bool enabled);

#endif                          