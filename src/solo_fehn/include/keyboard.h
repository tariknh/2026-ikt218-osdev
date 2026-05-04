/*
 * keyboard.h - PS/2 keyboard driver (Task 4 of Assignment 3)
 *
 * Reads scancodes from I/O port 0x60, translates them with a lookup table
 * and prints the resulting character to the terminal.
 */

#pragma once

void keyboard_install(void);
