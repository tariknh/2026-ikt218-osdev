#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>

void terminal_initialize();
void terminal_write(const char* data);
void terminal_write_dec(uint32_t v);
void terminal_update_cursor();
void terminal_move_left();
void terminal_move_right();
void terminal_refresh_line(char* buffer);
void terminal_save_screen();
void terminal_restore_screen();
extern int terminal_column;
extern int terminal_row;

#endif