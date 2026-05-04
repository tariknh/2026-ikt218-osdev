#ifndef SCREEN_H
#define SCREEN_H

void screen_clear(void); // Clears the screen
void screen_putc(char c); // Prints one character to the screen
void screen_write(const char* str); // Prints a string to the screen
void screen_write_hex(unsigned int n); // Prints a number in hexadecimal
void screen_write_dec(unsigned int n); // Prints a number in decimal


#endif