#pragma once

// Registers the IRQ1 handler and unmasks the keyboard IRQ line.
// Must be called after idt_init() and pic_init().
void keyboard_init(void);

// Drains the scancode ring buffer, translates each scancode to ASCII,
// and prints printable characters to the terminal.
// Call repeatedly from the main loop (e.g. after each hlt).
void keyboard_poll(void);

// Dequeues one scancode from the ring buffer and returns its ASCII character.
// Returns 0 if the buffer is empty or the key has no printable mapping.
char keyboard_getchar(void);
