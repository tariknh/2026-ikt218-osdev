#include "keyboard.h"
#include "irq.h"
#include "terminal.h"

/*
 * Hjelpefunksjon for å lese fra I/O-port.
 */
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/*
 * Scancode til ASCII lookup-tabell (US keyboard layout, set 1).
 */
static const char scancode_to_ascii[128] = {
    0,    27,  '1', '2', '3', '4', '5', '6',   /* 0x00 - 0x07 */
    '7', '8', '9', '0', '-', '=',  '\b', '\t', /* 0x08 - 0x0F */
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',    /* 0x10 - 0x17 */
    'o', 'p', '[', ']', '\n',  0,  'a', 's',   /* 0x18 - 0x1F */
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',    /* 0x20 - 0x27 */
    '\'', '`',  0, '\\', 'z', 'x', 'c', 'v',   /* 0x28 - 0x2F */
    'b', 'n', 'm', ',', '.', '/',  0,  '*',    /* 0x30 - 0x37 */
     0,  ' ',  0,   0,   0,   0,   0,   0,     /* 0x38 - 0x3F */
     0,   0,   0,   0,   0,   0,   0,   0,     /* 0x40 - 0x47 */
     0,   0,  '-',  0,   0,   0,  '+',  0,     /* 0x48 - 0x4F */
     0,   0,   0,   0,   0,   0,   0,   0,     /* 0x50 - 0x57 */
     0,   0,   0,   0,   0,   0,   0,   0,     /* 0x58 - 0x5F */
     0,   0,   0,   0,   0,   0,   0,   0,     /* 0x60 - 0x67 */
     0,   0,   0,   0,   0,   0,   0,   0,     /* 0x68 - 0x6F */
     0,   0,   0,   0,   0,   0,   0,   0,     /* 0x70 - 0x77 */
     0,   0,   0,   0,   0,   0,   0,   0,     /* 0x78 - 0x7F */
};

/* Enkel ringbuffer for å lagre tastatur-input. */
static char key_buffer[KEYBOARD_BUFFER_SIZE];
static uint8_t buffer_pos = 0;

/* -----------------------------------------------------------------
 * Polling state (added for Assignment 6 menu)
 * ----------------------------------------------------------------- */

/* Echo = auto-print pressed keys to terminal. Default: on. */
static bool echo_enabled = true;

/* Last scancode press that has not yet been consumed by get_scancode(). */
static volatile uint8_t last_scancode = 0;

/* Last ASCII char that has not yet been consumed by get_char(). */
static volatile char last_char = 0;

/* -----------------------------------------------------------------
 * IRQ 1 handler
 * ----------------------------------------------------------------- */

void keyboard_handler(struct isr_frame* frame) {
    (void)frame;

    uint8_t scancode = inb(KEYBOARD_DATA_PORT);

    /* Ignore key-release events (bit 7 set) */
    if (scancode & 0x80) {
        return;
    }

    /* Save scancode for polling */
    last_scancode = scancode;

    /* Translate to ASCII */
    char c = scancode_to_ascii[scancode];
    if (c != 0) {
        last_char = c;
    }

    /* If echo is enabled, print to terminal (original behaviour) */
    if (echo_enabled && c != 0) {
        terminal_putchar(c);

        /* Save in ring buffer */
        key_buffer[buffer_pos] = c;
        buffer_pos = (buffer_pos + 1) % KEYBOARD_BUFFER_SIZE;
    }
}

/* -----------------------------------------------------------------
 * Public API
 * ----------------------------------------------------------------- */

void keyboard_init(void) {
    irq_register_handler(1, keyboard_handler);
}

void keyboard_set_echo(bool enabled) {
    echo_enabled = enabled;

    /* When switching modes, clear stale state */
    last_scancode = 0;
    last_char = 0;
}

uint8_t keyboard_get_scancode(void) {
    uint8_t sc = last_scancode;
    last_scancode = 0;   /* consume it */
    return sc;
}

char keyboard_get_char(void) {
    char c = last_char;
    last_char = 0;       /* consume it */
    return c;
}