#include "kbd_map.h"
#include "terminal.h"
#include "memory.h"

static int shift_pressed = 0;
// The actual table lives here
unsigned char kbd_us[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',   
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',   
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0,       
 '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0, '*',    0,       
  ' ',   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,       
    0,   0,   0,   0, '-',   0,   0,   0, '+',   0,   0,   0,   0,   0,       
    0,   0,   0,   0,   0,   0
};
unsigned char kbd_us_upper[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',   
  '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',   
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~',   0,       
 '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',   0, '*',    0,       
  ' ',   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,       
    0,   0,   0,   0, '-',   0,   0,   0, '+',   0,   0,   0,   0,   0,       
    0,   0,   0,   0,   0,   0
};

// Your Buffer Logic
char kbd_buffer[256];
int kbd_pos = 0;

void keyboard_handler(struct registers *r) {

    uint8_t scancode = inb(0x60);



    if (scancode == 0x2A || scancode == 0x36) {

        shift_pressed = 1;

        return;

    }

    // Check for Shift Release (Break codes: Make + 0x80)

    if (scancode == 0xAA || scancode == 0xB6) {

        shift_pressed = 0;

        return;

    }



    // Ignore other "Key Release" events

    if (scancode & 0x80) {

        return;

    }



    // Pick the right table based on state

    char ascii;

    if (shift_pressed) {

        ascii = kbd_us_upper[scancode];

    } else {

        ascii = kbd_us[scancode];

    }



    if (scancode == SCAN_LEFT) {

        if (kbd_pos > 0) {

            kbd_pos--;

            terminal_move_left();

        }

        return;

    }

    if (scancode == SCAN_RIGHT) {

        // Only move right if there's a character to move over

        if (kbd_buffer[kbd_pos] != '\0') {

            kbd_pos++;

            terminal_move_right();

        }

        return;

    }



    if (ascii == '\b' && kbd_pos > 0) {

        kbd_pos--;

        // [Shift buffer left logic here]

       

        // Move cursor back visually

        terminal_column--;

       

        // Refresh the line from the new position

        terminal_refresh_line(&kbd_buffer[kbd_pos]);

    }

    else if (ascii == '\n') {

        kbd_buffer[kbd_pos] = '\0';  // null-terminate

        kbd_pos = 0;                

        terminal_putchar('\n');



    }

    if (ascii != 0 && ascii != '\b' && ascii != '\n') {

        // [Shift buffer right logic here]

        kbd_buffer[kbd_pos] = ascii;

       

        // Move cursor forward FIRST for insertion

        terminal_column++;

       

        // Refresh (this redraws the tail and moves cursor back to terminal_column)

        terminal_refresh_line(&kbd_buffer[kbd_pos + 1]);

       

        kbd_pos++;

    }

}