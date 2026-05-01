#include "../include/matrix_rain.h"

#include "../include/io.h"
#include "../include/keyboard.h"
#include "../include/pit.h"

/* Standard VGA text mode screen size. */
#define MATRIX_WIDTH 80
#define MATRIX_HEIGHT 25

/* Green shades for the Matrix effect. */
#define MATRIX_HEAD_COLOR 0x0A
#define MATRIX_TRAIL_COLOR 0x02
#define MATRIX_CLEAR_COLOR 0x00
#define MATRIX_TRAIL_MIN 4
#define MATRIX_TRAIL_RANGE 12
#define MATRIX_SPEED_MAX 3

static volatile char* const matrix_screen = (volatile char*)0xB8000;

static uint32_t matrix_seed = 0x1234ABCD;

typedef struct {
    int head_row;
    int trail_length;
    int fall_speed;
} MatrixColumn;

/* Very small pseudo-random generator so each column feels a little different. */
static uint32_t matrix_random(void) {
    matrix_seed = (matrix_seed * 1103515245u) + 12345u;
    return matrix_seed;
}

/* Draw one character directly into VGA text memory. */
static void matrix_draw_char(int row, int column, char character, char color) {
    int screen_offset;

    if (row < 0 || row >= MATRIX_HEIGHT || column < 0 || column >= MATRIX_WIDTH) {
        return;
    }

    screen_offset = (row * MATRIX_WIDTH + column) * 2;
    matrix_screen[screen_offset] = character;
    matrix_screen[screen_offset + 1] = color;
}

/* Fill the whole screen with black spaces before the animation starts. */
static void matrix_clear_screen(void) {
    int row;
    int column;

    for (row = 0; row < MATRIX_HEIGHT; row++) {
        for (column = 0; column < MATRIX_WIDTH; column++) {
            matrix_draw_char(row, column, ' ', MATRIX_CLEAR_COLOR);
        }
    }
}

/* Use printable ASCII characters so the rain looks noisy and lively. */
static char matrix_get_random_symbol(void) {
    return (char)('!' + (matrix_random() % 94));
}

static void matrix_reset_column(MatrixColumn* column) {
    column->head_row = -(int)(matrix_random() % MATRIX_HEIGHT);
    column->trail_length = MATRIX_TRAIL_MIN + (int)(matrix_random() % MATRIX_TRAIL_RANGE);
    column->fall_speed = 1 + (int)(matrix_random() % MATRIX_SPEED_MAX);
}

static int column_should_move(int frame_count, int fall_speed) {
    return (frame_count % fall_speed) == 0;
}

static void matrix_tick_column(int column_index, MatrixColumn* column) {
    int row_to_clear = column->head_row - column->trail_length;
    int trail_part;

    if (row_to_clear >= 0) {
        matrix_draw_char(row_to_clear, column_index, ' ', MATRIX_CLEAR_COLOR);
    }

    for (trail_part = 1; trail_part < column->trail_length; trail_part++) {
        int current_row = column->head_row - trail_part;
        if (current_row >= 0 && current_row < MATRIX_HEIGHT) {
            matrix_draw_char(current_row, column_index, matrix_get_random_symbol(), MATRIX_TRAIL_COLOR);
        }
    }

    if (column->head_row >= 0 && column->head_row < MATRIX_HEIGHT) {
        matrix_draw_char(column->head_row, column_index, matrix_get_random_symbol(), MATRIX_HEAD_COLOR);
    }

    column->head_row++;

    if (column->head_row - column->trail_length >= MATRIX_HEIGHT) {
        matrix_reset_column(column);
    }
}

void run_matrix_rain(void) {
    MatrixColumn columns[MATRIX_WIDTH];
    int frame_count = 0;
    int column;

    keyboard_set_mode(KEYBOARD_MODE_COMMAND);
    matrix_clear_screen();

    for (column = 0; column < MATRIX_WIDTH; column++) {
        matrix_reset_column(&columns[column]);
    }

    while (1) {
        if (keyboard_take_action() == KEY_ACTION_ESCAPE) {
            return;
        }

        for (column = 0; column < MATRIX_WIDTH; column++) {
            if (column_should_move(frame_count, columns[column].fall_speed)) {
                matrix_tick_column(column, &columns[column]);
            }
        }

        frame_count++;
        sleep_interrupt(50);
    }
}
