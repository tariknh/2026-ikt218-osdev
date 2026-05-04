#pragma once

#define SNAKE_INPUT_UP 1
#define SNAKE_INPUT_DOWN 2
#define SNAKE_INPUT_LEFT 3
#define SNAKE_INPUT_RIGHT 4
#define SNAKE_INPUT_ESCAPE 27

int snake_is_running(void);
void snake_handle_input(char input);
void snake_run(void);
