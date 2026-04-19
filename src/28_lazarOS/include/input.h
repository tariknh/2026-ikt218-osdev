#ifndef INPUT_H
#define INPUT_H

#include <libc/stdint.h>

/*
 * Keyboard input ring buffer.
 * isr.c calls input_put_char() on every key-press IRQ.
 * User code calls getchar() / getline() to read input.
 */

#define INPUT_BUFFER_SIZE 256

/* Must be called once at boot before any keyboard input */
void input_init(void);

/* Called from irq_handler (isr.c) to push a character into the buffer */
void input_put_char(char c);

/* Blocking read: halts the CPU until a character is available */
char getchar(void);

/* Non-blocking: returns the next character, or 0 if the buffer is empty */
char getchar_nonblock(void);

/* Blocking line read: fills buf until Enter is pressed.
 * Characters are echoed to screen. Returns the number of characters read.
 * max_len does NOT include the null terminator (buf must be max_len+1). */
int getline(char *buf, int max_len);

/* Check whether a key is waiting in the buffer */
int input_available(void);

#endif /* INPUT_H */
