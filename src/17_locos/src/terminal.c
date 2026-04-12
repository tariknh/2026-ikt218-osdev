#include "terminal.h" //Includes header for terminal.h containing function declarations for best practice
#include <libc/stdarg.h> //Includes stdarg.h, library already here when we started the project
#include <libc/stdint.h> //Includes stdint.h, library already here when we started the project

#define VGA_ADDRESS 0xB8000 // Memory address for VGA needed to write directly to video memory for text output
#define VGA_COLS 80 // Defines the screen width as 80 characters
#define VGA_ROWS 25 // Defines screen heigh as 25 rows
#define VGA_COLOR 0x1F // Sets text attributes: 0x1F which is white text on blue background (can be changed in future)

static volatile uint16_t* vga = (uint16_t*)VGA_ADDRESS; // Pointer to VGA memory that wont be cached by compiler
static int cursor_x = 0; // Horizontal cursot position
static int cursor_y = 0; // Vertical cursor position

void terminal_init(void) { //A function used in kernel.c to initialize the terminal subsystem so text output works
    for (int i = 0; i < VGA_COLS * VGA_ROWS; i++) { // Loop through all character cells on the screen
        vga[i] = (uint16_t)(VGA_COLOR << 8) | ' '; //Writes color and space char to each cell clearing the screen
    }

    cursor_x = 0; //resets cursor position
    cursor_y = 0; //resets cursor position
}

static void put_char(char c) { // Helper function to write a single character
    if (c == '\n') { // Handles newline specially
        cursor_x = 0; // Resets horizontal position to start of line
        cursor_y++; // Moves down to next line
        return; 
    }

    if (cursor_y >= VGA_ROWS) { //Checks if cursor is off bottom of screen
        return;
    }

    vga[cursor_y * VGA_COLS + cursor_x] = (uint16_t)(VGA_COLOR << 8) | (uint8_t)c; // Calculate linear offsett into VGA and writes character and color

    cursor_x++; //Advances cursor horizontally
    if (cursor_x >= VGA_COLS) { //Checks if cursor reached end of line
        cursor_x = 0; // Resets horizontal position to start of line
        cursor_y++; // Moves down to next line
    }
}

void terminal_write(const char* str) { //A null-terminated string is outputted
    while (*str) { // iterates every character in string
        put_char(*str++); //Outputs currect character and advances pointer
    }
}

static void utoa(unsigned int value, unsigned int base, char* buf) { // Help function converting unsigned integer to string
    static const char digits[] = "0123456789abcdef"; // Lookup table for hex/decimal digit characters
    char tmp[32]; //temporary buffer for reversed digits
    int i = 0; // Counter for number of digits extracted

    if (value == 0) { // Special case if its zero
        buf[0] = '0'; // Write 0 to buffer output
        buf[1] = '\0'; // Null terminates
        return;
    }

    while (value && i < (int)sizeof(tmp)) { //Extrcts digits by repeated modulo/divison
        tmp[i++] = digits[value % base]; //appends next digit from lookup
        value /= base; //removes the last extracted digit
    }

    for (int j = 0; j < i; j++) { //reverses digits from tmp into buf (because it was right to left extraction)
        buf[j] = tmp[i - j - 1]; // Copies these reverse digits
    }
    buf[i] = '\0'; // Null terminates the output string
}

void terminal_printf(const char* fmt, ...) { //Variadic function for formatted output just like C printf
    va_list ap; // DEclares a list to traverse variadic arguments
    char numbuf[32]; // Buffer for integer to string conversion

    va_start(ap, fmt); // Initializes ap to point to first variadic argument after the fmt parameter

    while (*fmt) { //Iterates through format string
        if (*fmt != '%') { //Checks for format specifier
            put_char(*fmt++); //Outputs literal character and advances
            continue; // skips to next format character
        }

        fmt++; // Advances past % to read formatted code
        switch (*fmt) { //Handles format specifiers here with different cases for each type
            case '%': // Case for escaped percent sign %%
                put_char('%'); // Outputs a single % character
                break; // Ends this case
            case 'c':
                put_char((char)va_arg(ap, int)); // retrieves and outputs next variadic argument as character
                break;
            case 's': {
                const char* s = va_arg(ap, const char*); //Retrieves next variadic argument as string pointer
                if (s) { //Checks for null pointer
                    terminal_write(s); // Outputs non-null string
                } else {
                    terminal_write("(null)"); // Outputs null if string pointer was null to indicate this in output
                }
                break;
            }
            case 'd': {
                int v = va_arg(ap, int); // Retrieves next variadic argument as signed integer
                unsigned int uv = (v < 0) ? (unsigned int)(-v) : (unsigned int)v; //converts to unsigned for utoa
                if (v < 0) { // Outputs minus sign if negative
                    put_char('-');
                }
                utoa(uv, 10, numbuf); // converts unsigned magnitude to decimal string
                terminal_write(numbuf); // Outputs the digit string
                break;
            }
            case 'u': {
                unsigned int v = va_arg(ap, unsigned int); // Retrieves next variadic argument as unsigned integer
                utoa(v, 10, numbuf); 
                terminal_write(numbuf);
                break;
            }
            case 'x': { // The same as above different decimal
                unsigned int v = va_arg(ap, unsigned int);
                utoa(v, 16, numbuf);
                terminal_write(numbuf);
                break;
            }
            default: // Handles unknown format specifiers
                put_char('%'); // Outputs the literal % character
                put_char(*fmt); // Outputs the unknown specifier as is to indicate it was not recognized
                break;
        }

        if (*fmt) { //Advances the format pointer if not at null terminator already
            fmt++; 
        }
    }

    va_end(ap); // Clean up macro for variadic argument list
}
