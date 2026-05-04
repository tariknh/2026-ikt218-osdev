#pragma once

#include "stdbool.h"

void init_isr(void);

bool keyboard_try_read(char *out);

char keyboard_wait_read(void);
