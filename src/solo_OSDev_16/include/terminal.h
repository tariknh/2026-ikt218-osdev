#ifndef TERMINAL_H 
#define TERMINAL_H 


#include <libc/stddef.h>
#include <libc/stdint.h>

void terminal_initialize(void);         // Initializing the terminal
void terminal_putchar(char c);          // Write one charecter to the terminal 
void terminal_write(const char* data);  // Write a null-terminated string to the terminal

#endif 