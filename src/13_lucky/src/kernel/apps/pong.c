#include "kernel/apps.h"

#include "arch/i386/isr.h"
#include "arch/i386/keyboard.h"
#include "kernel/pit.h"
#include "stdbool.h"
#include "stdio.h"
#include "song/frequencies.h"
#include "song/song.h"

#define PONG_WIDTH 70
#define PONG_HEIGHT 20
#define PONG_LEFT_PADDLE_X 3
#define PONG_RIGHT_PADDLE_X (PONG_WIDTH - 4)
#define PONG_PADDLE_HEIGHT 5

void run_pong(void) {
    int ball_x = 20;
    int ball_y = 10;
    int dx = 1;
    int dy = 1;
    int paddle_y = 7;
    char key = 0;

    while (true) {
        terminal_clear();
        printf("=== Pong ===       W/S: move       ESC: menu\n\n");

        // Draw one full frame into VGA text mode
        for (int y = 0; y < PONG_HEIGHT; y++) {
            for (int x = 0; x < PONG_WIDTH; x++) {
                if (y == 0 || y == PONG_HEIGHT - 1) {
                    putchar('-');
                } else if (x == 0 || x == PONG_WIDTH - 1) {
                    putchar('|');
                } else if (x == ball_x && y == ball_y) {
                    putchar('O');
                } else if (x == PONG_LEFT_PADDLE_X && y >= paddle_y && y < paddle_y + PONG_PADDLE_HEIGHT) {
                    putchar('|');
                } else if (x == PONG_RIGHT_PADDLE_X && y >= ball_y - 2 && y <= ball_y + 2) {
                    putchar('|');
                } else {
                    putchar(' ');
                }
            }
            putchar('\n');
        }

        while (keyboard_try_read(&key)) {
            if (key == KEYBOARD_SCANCODE_TO_CHAR[KEY_SCANCODE_ESCAPE]) {
                return;
            }

            if ((key == 'w' || key == 'W') && paddle_y > 1) {
                paddle_y--;
            }

            if ((key == 's' || key == 'S') && paddle_y < PONG_HEIGHT - PONG_PADDLE_HEIGHT - 1) {
                paddle_y++;
            }
        }

        ball_x += dx;
        ball_y += dy;

        // Player paddle hit
        if (ball_x == PONG_LEFT_PADDLE_X + 1 && dx < 0 &&
            ball_y >= paddle_y && ball_y < paddle_y + PONG_PADDLE_HEIGHT) {
            dx = -dx;
            play_tone(C5, 25);
        } else if (ball_x <= 1) {
            ball_x = PONG_WIDTH / 2;
            ball_y = PONG_HEIGHT / 2;
            dx = 1;
            play_tone(C5, 25);
        }

        // The right paddle follows the ball
        if (ball_x >= PONG_RIGHT_PADDLE_X - 1 && dx > 0) {
            dx = -dx;
            play_tone(C5, 25);
        } else if (ball_x >= PONG_WIDTH - 2) {
            dx = -dx;
            play_tone(C5, 25);
        }

        // Bounce off the top and bottom walls
        if (ball_y <= 1 || ball_y >= PONG_HEIGHT - 2) {
            dy = -dy;
            play_tone(C5, 25);
        }

        sleep_interrupt(75);
    }
}
