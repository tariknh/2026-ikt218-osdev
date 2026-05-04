#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <libc/stdint.h>
#include <libc/stdbool.h>

void keyboard_handle_irq(void);

/*
 * Returns last typed key and clears it.
 */
char keyboard_get_last_key(void);

/*
 * Enables or disables automatic keyboard echo to the terminal.
 */
void keyboard_set_echo(bool enabled);

#endif
