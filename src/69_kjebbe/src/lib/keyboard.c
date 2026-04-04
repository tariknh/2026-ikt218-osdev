#include "../../include/keyboard.h"
#include "../../include/fedrelandet.h"
#include "../../include/io.h"
#include "../../include/irq.h"
#include "../../include/libc/stdint.h"
#include "../../include/libc/stdio.h"
#include "../../include/menu.h"
#include "../../include/piano.h"
#include "../../include/program.h"
#include "../../include/radio.h"
#include "../../include/shell.h"
#include "song/song.h"

// Maps scancode to character. keyboard Layout: US QWERTY
const char scancode_ascii[128] = {
    0,   0,    '1',  '2', '3',  '4', '5', '6', '7', '8', '9', '0', '-',
    '=', '\b', '\t', 'q', 'w',  'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
    '[', ']',  '\n', 0,   'a',  's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',
    ';', '\'', '`',  0,   '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',',
    '.', '/',  0,    '*', 0,    ' ', 0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,    0,   '7', '8', '9', '-', '4', '5', '6',
    '+', '1',  '2',  '3', '0',  '.', 0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,    0,   0,   0,   0,   0,   0,
};

// Function to add element to circular queue.
void kb_enqueue(KeyboardBuffer *kb, int entry) {
  kb->buffer[kb->front] = entry;
  kb->front = (kb->front + 1) % KEYBOARD_BUFFER_SIZE;
}
// Function to retrieve element to circular queue.
int kb_dequeue(KeyboardBuffer *kb) {
  if (kb->front == kb->back) {
    return -1;
  }
  int entry = kb->buffer[kb->back];
  kb->back = (kb->back + 1) % KEYBOARD_BUFFER_SIZE;
  return entry;
}

KeyboardBuffer kb = {0};

// Gets keyboard key, returning -1 on key release.
int get_key(int scancode) {
  if (scancode & 0x80)
    return -1;
  return scancode_ascii[scancode];
}

// Routes scancode to appropriate handler based on current active program.
void keyboard_handler(int scancode) {

  switch (active_program) {
  case PROGRAM_SHELL:
    shell_keyboard_handler(scancode);
    break;
  case PROGRAM_PIANO:
    piano_keyboard_handler(scancode);
    break;
  case PROGRAM_RADIO:
    radio_keyboard_handler(scancode);
    break;
  case PROGRAM_MENU:
    menu_keyboard_handler(scancode);
    break;
  case PROGRAM_FEDRELANDET:
    fedrelandet_keyboard_handler(scancode);
    break;
  default:
    printf("unknown program: %d\n", active_program);
    break;
  }
}

// handler for IRQ1 (keyboard)
static void keyboard_irq_handler(registers_t *regs) {
  (void)regs;

  uint8_t scancode =
      inb(KEYBOARD_DATA_PORT); // Read scancode from keyboard data port

  // If 'q' is pressed set program to menu (quits current program)
  if (scancode == 0x10) {
    active_program = PROGRAM_MENU;
    disable_speaker();
    print_menu();
    return;
  }

  // queues keyboard scancode to ring buffer
  kb_enqueue(&kb, scancode);
}

// Maps IRQ 1 to the pointer of the keyboard handler function.
void keyboard_init(void) { irq_register_handler(1, keyboard_irq_handler); }
