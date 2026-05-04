/*
 * keyboard.c - PS/2 keyboard driver
 *
 * For Assignment 6 the keyboard no longer prints characters directly to
 * the terminal.  Instead it forwards each character to the shell, which
 * does its own echoing and line-buffering.
 */

#include <keyboard.h>
#include <shell.h>
#include <irq.h>
#include <io.h>
#include <libc/stdint.h>

#define KBD_DATA_PORT  0x60

/* US-layout scancode set 1 -> ASCII.  0x00 means "no character to print". */
static const char scancode_ascii[128] = {
    0,    27,  '1', '2', '3', '4', '5', '6', '7', '8',
    '9',  '0', '-', '=', '\b','\t','q', 'w', 'e', 'r',
    't',  'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
    'a',  's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'', '`', 0,   '\\','z', 'x', 'c', 'v', 'b', 'n',
    'm',  ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,
};

static void keyboard_callback(registers_t* r) {
    (void)r;

    uint8_t scancode = inb(KBD_DATA_PORT);

    /* Top bit set = key release.  Skip those. */
    if (scancode & 0x80) {
        return;
    }

    char c = (scancode < 128) ? scancode_ascii[scancode] : 0;
    if (c != 0) {
        shell_on_char(c);
    }
}

void keyboard_install(void) {
    irq_register_handler(1, keyboard_callback);
    irq_unmask(1);
}
