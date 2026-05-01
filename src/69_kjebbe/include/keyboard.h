#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "../include/libc/stdint.h"
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_BUFFER_SIZE 128

// Stores keyboard scancodes in circular queue to be processed by the main
// kernel loop.
typedef struct {
  int buffer[KEYBOARD_BUFFER_SIZE];
  int front;
  int back;
} KeyboardBuffer;

extern KeyboardBuffer kb;
extern const char scancode_ascii[128];

void kb_enqueue(KeyboardBuffer *kb, int entry);
int kb_dequeue(KeyboardBuffer *kb);
void keyboard_init(void);

int get_key(int scancode);
void keyboard_handler(int scancode);

#endif
