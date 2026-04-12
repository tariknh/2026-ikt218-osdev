/*
Name: terminal.h
Project: LocOS
Description: This file contains the function declarations for the terminal subsystem of the LocOS kernel.
*/

#ifndef TERMINAL_H
#define TERMINAL_H

void terminal_init(void); // Declares function to initialize the terminal subsystem, you can find hte insides in terminal.c
void terminal_write(const char* str); // Declares function to write a null-terminated string to the terminal, you can find the insides in terminal.c
void terminal_printf(const char* fmt, ...); // Declares a variadic function for formatted output to the terminal, similar to C's printf. The implementation is in terminal.c

#endif
