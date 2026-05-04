#ifndef SNAKE_H
#define SNAKE_H

#include "stdint.h"

#define SNAKE_BOARD_WIDTH  40
#define SNAKE_BOARD_HEIGHT 20
#define SNAKE_MAX_LENGTH   128

typedef enum {
    SNAKE_DIR_UP = 0,
    SNAKE_DIR_DOWN,
    SNAKE_DIR_LEFT,
    SNAKE_DIR_RIGHT
} SnakeDirection;

typedef struct {
    int x[SNAKE_MAX_LENGTH];
    int y[SNAKE_MAX_LENGTH];
    int length;

    int food_x;
    int food_y;

    SnakeDirection direction;
    int score;
    int game_over;
    int initialized;
} SnakeGame;

void snake_init(void);
void snake_draw(void);
void snake_update(void);
void snake_handle_key(char key);
int  snake_is_running(void);

#endif
