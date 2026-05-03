#ifndef TERMINAL_H 
#define TERMINAL_H 


#ifdef __cplusplus
extern "C"{
#endif


#include <libc/stddef.h>
#include <libc/stdint.h>

void terminal_initialize(void);         // Initializing the terminal
void terminal_putchar(char c);          // Write one charecter to the terminal 
void terminal_write(const char* data);  // Write a null-terminated string to the terminal
void terminal_clear();                  // Wrapper for terminal_initialize, for clarification 
void print_hex(uint32_t value);         // Print hexadecimal numbers
void print_dec(uint32_t value);         // Print decimal numbers


#ifdef __cplusplus
}
#endif
#endif 