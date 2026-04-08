#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>

void terminal_initialize();
void terminal_write(const char* data);
void terminal_write_dec_at();
extern int terminal_column;
extern int terminal_row;

#endif