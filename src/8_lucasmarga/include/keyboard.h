#pragma once

#include "libc/stdint.h"

void keyboard_handle_input(void);

char keyboard_get_key(void);
char keyboard_wait_key(void);
void keyboard_clear_buffer(void);
void keyboard_set_echo(uint8_t enabled);