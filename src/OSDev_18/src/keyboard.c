#include <kernel/keyboard.h>
#include <kernel/interrupt.h>
#include <kernel/io.h>
#include <kernel/terminal.h>
#include <libc/stdint.h>

static uint32_t index = 0;
static uint8_t keyboardBuffer[KEYBOARD_BUFFER_SIZE];

static volatile char lastKeyPressed = 0;

static const char scancodeToAscii[128] = {
    0,   27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,   'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0,   '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0,   ' ', 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+',
    '1', '2', '3', '0', '.',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

char GetLastKeyPressed(void) {
    // Consume the last translated key once so polling loops react to each
    // press a single time instead of repeatedly seeing the same key.
    char key = lastKeyPressed;
    lastKeyPressed = 0;
    return key;
}

void KeyboardHandler(struct Registers* registers) {
    (void) registers; // cast to void to silence warning

    uint8_t scancode = InPortByte(KEYBOARD_DATA_PORT);

    if (index < KEYBOARD_BUFFER_SIZE) {
        keyboardBuffer[index] = scancode;
        index++;
    }

    // ignores key release events
    if (scancode & 0x80) {
        return;
    }

    if (scancode < 128) {
        char ascii = scancodeToAscii[scancode];

        if (ascii != 0) {
            TerminalPutChar(ascii);
            lastKeyPressed = ascii;
        }
    }
}
