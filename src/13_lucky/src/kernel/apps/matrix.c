#include "kernel/apps.h"

#include "arch/i386/isr.h"
#include "arch/i386/keyboard.h"
#include "kernel/pit.h"
#include "stdbool.h"
#include "stdio.h"
#include "stdint.h"

#define MATRIX_WIDTH 78
#define MATRIX_HEIGHT 23
#define MATRIX_TAIL 10

static const char symbols[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static uint32_t random_state = 0xC0FFEE;

// Random generator (LCG) similar to the one from glibc
static uint32_t random(void) {
    random_state = random_state * 1103515245 + 12345;
    return random_state;
}

void run_matrix_effect(void) {
    int columns[MATRIX_WIDTH];
    char key = 0;

    for (int x = 0; x < MATRIX_WIDTH; x++) {
        columns[x] = -(int) (random() % MATRIX_HEIGHT);
    }

    while (true) {
        while (keyboard_try_read(&key)) {
            if (key == KEYBOARD_SCANCODE_TO_CHAR[KEY_SCANCODE_ESCAPE]) {
                return;
            }
        }

        terminal_clear();
        printf("=== Matrix rain ===       ESC: menu\n");

        for (int y = 0; y < MATRIX_HEIGHT; y++) {
            for (int x = 0; x < MATRIX_WIDTH; x++) {
                if (y <= columns[x] && y > columns[x] - MATRIX_TAIL) {
                    putchar(symbols[random() % (sizeof(symbols) - 1)]);
                } else {
                    putchar(' ');
                }
            }
            putchar('\n');
        }

        for (int x = 0; x < MATRIX_WIDTH; x++) {
            columns[x]++;
            if (columns[x] - MATRIX_TAIL > MATRIX_HEIGHT) {
                columns[x] = -(int) (random() % MATRIX_HEIGHT);
            }
        }

        sleep_interrupt(50);
    }
}
