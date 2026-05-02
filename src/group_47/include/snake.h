#ifndef SNAKE_H
#define SNAKE_H

#include "libc/stdint.h"

// Game dimensions (adjust to your terminal size)
#define SNAKE_WIDTH  80
#define SNAKE_HEIGHT 25

#define MAX_SNAKE_LENGTH 100

// Direction enum for clarity and type safety
typedef enum {
    DIR_UP,
    DIR_RIGHT,
    DIR_DOWN,
    DIR_LEFT
} direction_t;

// Snake object definition 
typedef struct {
    int x[MAX_SNAKE_LENGTH];
    int y[MAX_SNAKE_LENGTH];
    int length;
    direction_t dir;
} snake_t;

// Global direction (controlled by keyboard)
extern volatile direction_t snake_dir;

// Public functions
void snake_start();
void snake_set_direction(direction_t dir);

#endif