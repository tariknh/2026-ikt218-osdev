#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "libc/stdint.h"
#include "libc/stdbool.h"
#include "isr.h"

#define KEYBOARD_DATA_PORT     0x60
#define KEYBOARD_BUFFER_SIZE   256

/* Common scancodes (set 1) */
#define KEY_ESC        0x01
#define KEY_ENTER      0x1C
#define KEY_UP         0x48
#define KEY_DOWN       0x50
#define KEY_LEFT       0x4B
#define KEY_RIGHT      0x4D
#define KEY_SPACE      0x39
#define KEY_BACKSPACE  0x0E
#define KEY_TAB        0x0F

/* Initialize keyboard driver (registers IRQ1 handler). */
void keyboard_init(void);

/* Internal IRQ handler - do not call directly. */
void keyboard_handler(struct isr_frame* frame);

void keyboard_set_echo(bool enabled);


uint8_t keyboard_get_scancode(void);

char keyboard_get_char(void);

#endif /* KEYBOARD_H */