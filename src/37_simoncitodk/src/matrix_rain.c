#include "idt.h"
#include "pit.h"
#include "terminal.h"
#include <libc/stdint.h>

#define MATRIX_WIDTH 80
#define MATRIX_HEIGHT 25
#define MATRIX_STATUS_ROW 0
#define MATRIX_FIRST_RAIN_ROW 4
#define MATRIX_TRAIL_LENGTH 6

#define MATRIX_SPEED_SLOW_MS 120
#define MATRIX_SPEED_NORMAL_MS 60
#define MATRIX_SPEED_FAST_MS 25

static uint32_t matrix_random_seed = 12345;

static uint32_t matrix_random(void)
{
    matrix_random_seed = matrix_random_seed * 1103515245 + 12345;
    return matrix_random_seed;
}

static char matrix_random_char(void)
{
    const char symbols[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    uint32_t index = matrix_random() % 36;
    return symbols[index];
}

static void matrix_write_text_at(const char *text, int column, int row, uint8_t color)
{
    int i = 0;

    while (text[i] != '\0') {
        terminal_putchar_at(text[i], color, column + i, row);
        i++;
    }
}

static void matrix_clear_row(int row)
{
    for (int column = 0; column < MATRIX_WIDTH; column++) {
        terminal_putchar_at(' ', 0x00, column, row);
    }
}

static void matrix_draw_status(uint32_t frame_delay_ms)
{
    matrix_clear_row(0);
    matrix_clear_row(1);
    matrix_clear_row(2);

    matrix_write_text_at("Matrix Rain", 0, MATRIX_STATUS_ROW, 0x0A);
    matrix_write_text_at("Keyboard: 1 slow | 2 normal | 3 fast | q quit", 0, 1, 0x07);

    if (frame_delay_ms == MATRIX_SPEED_SLOW_MS) {
        matrix_write_text_at("Current speed: slow", 0, 2, 0x07);
    } else if (frame_delay_ms == MATRIX_SPEED_FAST_MS) {
        matrix_write_text_at("Current speed: fast", 0, 2, 0x07);
    } else {
        matrix_write_text_at("Current speed: normal", 0, 2, 0x07);
    }
}

static int matrix_should_quit(void)
{
    return keyboard_get_last_key() == 'q';
}

static uint32_t matrix_update_speed_from_keyboard(uint32_t current_delay_ms)
{
    char key = keyboard_get_last_key();

    if (key == '1') {
        return MATRIX_SPEED_SLOW_MS;
    }

    if (key == '2') {
        return MATRIX_SPEED_NORMAL_MS;
    }

    if (key == '3') {
        return MATRIX_SPEED_FAST_MS;
    }

    return current_delay_ms;
}

static void matrix_show_startup_status(void)
{
    terminal_clear();

    matrix_write_text_at("Matrix Rain", 0, MATRIX_STATUS_ROW, 0x0A);
    matrix_write_text_at("GDT IDT PIC Memory Paging PIT VGA Keyboard: OK", 0, 1, 0x07);
    matrix_write_text_at("Starting timed VGA animation...", 0, 2, 0x07);

    sleep_interrupt(1200);

    terminal_clear();
}

void matrix_rain_demo(void)
{
    int column_heads[MATRIX_WIDTH];
    uint32_t frame_delay_ms = MATRIX_SPEED_NORMAL_MS;

    matrix_show_startup_status();
    matrix_draw_status(frame_delay_ms);

    for (int column = 0; column < MATRIX_WIDTH; column++) {
        column_heads[column] = MATRIX_FIRST_RAIN_ROW + (int)(matrix_random() % 8);
    }

    for (;;) {
        if (matrix_should_quit()) {
            return;
        }

        frame_delay_ms = matrix_update_speed_from_keyboard(frame_delay_ms);
        matrix_draw_status(frame_delay_ms);

        for (int column = 0; column < MATRIX_WIDTH; column++) {
            int head_row = column_heads[column];

            int clear_row = head_row - MATRIX_TRAIL_LENGTH;
            if (clear_row >= MATRIX_FIRST_RAIN_ROW && clear_row < MATRIX_HEIGHT) {
                terminal_putchar_at(' ', 0x00, column, clear_row);
            }

            int trail_row = head_row - 1;
            if (trail_row >= MATRIX_FIRST_RAIN_ROW && trail_row < MATRIX_HEIGHT) {
                terminal_putchar_at(matrix_random_char(), 0x02, column, trail_row);
            }

            if (head_row >= MATRIX_FIRST_RAIN_ROW && head_row < MATRIX_HEIGHT) {
                terminal_putchar_at(matrix_random_char(), 0x0A, column, head_row);
            }

            column_heads[column]++;

            if (column_heads[column] >= MATRIX_HEIGHT + MATRIX_TRAIL_LENGTH) {
                column_heads[column] = MATRIX_FIRST_RAIN_ROW + (int)(matrix_random() % 8);
            }
        }

        sleep_interrupt(frame_delay_ms);
    }
}
