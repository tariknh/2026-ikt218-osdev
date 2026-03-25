#ifndef KBD_MAP_H
#define KBD_MAP_H

// This tells other files that kbd_us exists somewhere else
extern unsigned char kbd_us[128];
extern unsigned char kbd_us_upper[128];

// It's also a great place to define special keys
#define KBD_DATA_PORT 0x60
#define BACKSPACE 0x0E
#define ENTER 0x1C
#define LSHIFT 0x2A
#define RSHIFT 0x36

#endif