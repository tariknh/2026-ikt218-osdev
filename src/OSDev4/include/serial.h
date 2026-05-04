#pragma once

void serial_init(void);
void serial_putchar(char c);
void serial_write(const char* str);
int  serial_data_ready(void);
char serial_getchar(void);
