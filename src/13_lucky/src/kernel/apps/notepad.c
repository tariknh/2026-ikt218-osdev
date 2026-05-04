#include "kernel/apps.h"

#include "arch/i386/isr.h"
#include "arch/i386/keyboard.h"
#include "stdbool.h"
#include "stdio.h"
#include "stdint.h"

#define NOTEPAD_BUFFER_SIZE 1600

static char notepad_buffer[NOTEPAD_BUFFER_SIZE];
static uint32_t notepad_length = 0;

// Redraw from the saved buffer after edits
// This is done to support backspace
static void draw_notepad(void) {
    terminal_clear();
    printf("=== Notepad ===       ESC: menu\n");

    for (uint32_t i = 0; i < notepad_length; i++) {
        putchar(notepad_buffer[i]);
    }
}

void run_notepad(void) {
    draw_notepad();

    while (true) {
        char key = keyboard_wait_read();

        if (key == KEYBOARD_SCANCODE_TO_CHAR[KEY_SCANCODE_ESCAPE]) {
            return;
        }

        if (key == '\b') {
            if (notepad_length > 0) {
                notepad_length--;
                notepad_buffer[notepad_length] = '\0';
                draw_notepad();
            }
            continue;
        }

        if (notepad_length < NOTEPAD_BUFFER_SIZE - 1) {
            notepad_buffer[notepad_length] = key;
            notepad_length++;
            notepad_buffer[notepad_length] = '\0';
            putchar(key);
        }
    }
}
