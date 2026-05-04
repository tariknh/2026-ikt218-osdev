#ifndef SNAKE_H
#define SNAKE_H

#include <libc/stdint.h>

#define GAME_WIDTH 80
#define GAME_HEIGHT 23
#define MAX_SNAKE_LENGTH 500

#define DIR_UP 0
#define DIR_DOWN 1
#define DIR_LEFT 2
#define DIR_RIGHT 3

typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    Point body[MAX_SNAKE_LENGTH];
    int length;
    int direction;
    int alive;
} Snake;

typedef struct {
    Snake snake;
    Point food;
    int score;
    uint32_t tick_rate;
} Game;

void snake_game(void);
void snake_key_handler(uint8_t scancode);
void snake_restart_handler(uint8_t scancode);

extern volatile int restart_pressed;

#endif