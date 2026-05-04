#include <libc/stdint.h>
#include "keyboard.h"
#include "io.h"
#include "snake.h"
#include "shell.h"

#define KBD_DATA_PORT 0x60
#define PIC1_DATA_PORT 0x21

static uint8_t extended_scancode_pending = 0;

void keyboard_init(void) {
    uint8_t mask = inb(PIC1_DATA_PORT);
    outb(PIC1_DATA_PORT, (uint8_t)(mask & ~(1u << 1)));
}

static char scancode_to_ascii(uint8_t sc) {
    switch (sc) {
        case 0x01: return (char)SNAKE_INPUT_ESCAPE;
        case 0x02: return '1';
        case 0x03: return '2';
        case 0x04: return '3';
        case 0x05: return '4';
        case 0x06: return '5';
        case 0x07: return '6';
        case 0x08: return '7';
        case 0x09: return '8';
        case 0x0A: return '9';
        case 0x0B: return '0';

        case 0x10: return 'q';
        case 0x11: return 'w';
        case 0x12: return 'e';
        case 0x13: return 'r';
        case 0x14: return 't';
        case 0x15: return 'y';
        case 0x16: return 'u';
        case 0x17: return 'i';
        case 0x18: return 'o';
        case 0x19: return 'p';

        case 0x1E: return 'a';
        case 0x1F: return 's';
        case 0x20: return 'd';
        case 0x21: return 'f';
        case 0x22: return 'g';
        case 0x23: return 'h';
        case 0x24: return 'j';
        case 0x25: return 'k';
        case 0x26: return 'l';

        case 0x2C: return 'z';
        case 0x2D: return 'x';
        case 0x2E: return 'c';
        case 0x2F: return 'v';
        case 0x30: return 'b';
        case 0x31: return 'n';
        case 0x32: return 'm';

        case 0x39: return ' ';
        case 0x1C: return '\n'; // Enter
        case 0x0E: return '\b'; // Backspace

        default:   return 0;
    }
}

static char extended_scancode_to_input(uint8_t sc) {
    switch (sc) {
        case 0x48: return (char)SNAKE_INPUT_UP;
        case 0x50: return (char)SNAKE_INPUT_DOWN;
        case 0x4B: return (char)SNAKE_INPUT_LEFT;
        case 0x4D: return (char)SNAKE_INPUT_RIGHT;
        default: return 0;
    }
}

void keyboard_on_irq1(void) {
    uint8_t sc = inb(KBD_DATA_PORT);

    if (sc == 0xE0) {
        extended_scancode_pending = 1;
        return;
    }

    if (extended_scancode_pending) {
        extended_scancode_pending = 0;

        char c = extended_scancode_to_input(sc);
        if (c != 0) {
            shell_handle_char(c);
        }
        return;
    }

    // Ignore key release events
    if (sc & 0x80) {
        return;
    }

    char c = scancode_to_ascii(sc);
    if (c != 0) {
        shell_handle_char(c);
    }
}