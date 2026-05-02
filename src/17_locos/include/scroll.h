#ifndef SCROLL_H
#define SCROLL_H

#include <libc/stdint.h>

void scroll_start(const char *text);
void scroll_stop(void);
void scroll_on_tick(uint32_t tick);

#endif