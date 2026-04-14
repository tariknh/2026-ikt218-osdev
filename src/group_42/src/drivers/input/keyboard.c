#include "drivers/input/keyboard.h"

#include <drivers/input/keymap.h>
#include <kernel/log.h>
#include <stdbool.h>
#include <stdio.h>

#include "arch/i386/cpu/isr.h"
#include "arch/i386/cpu/ports.h"
#include "kernel/input.h"
#include "kernel/util/bitops.h"
#include "kernel/util/ring_buffer8.h"

#define PS2_DATA_PORT 0x60
#define PS2_COMMAND_PORT 0x64

#define PS2_CMD_READ_CONFIG 0x20
#define PS2_CMD_WRITE_CONFIG 0x60

#define KB_CMD_SET_SCANCODE_SET 0xF0
#define KB_CMD_ACK 0xFA

#define PS2_STATUS_IN_BUF_FULL (1 << 1)


#define SCAN_PREFIX_EO 0xE0
#define SCAN_RELEASE 0xF0


#define KEYBOARD_BUFFER_SIZE 256

enum modifier_bit {
  MODIFIER_LSHIFT = 0,
  MODIFIER_RSHIFT = 1,
  MODIFIER_LCTRL = 2,
  MODIFIER_RCTRL = 3,
  MODIFIER_LALT = 4,
  MODIFIER_RALT = 5,
  MODIFIER_CAPSLOCK = 6,
};
enum kb_scancode {
  KB_LSHIFT = 0x12, // release = F0
  KB_RSHIFT = 0x59, // release = F0

  KB_CTRL = 0x14,     // release = F0, right = E0.
  KB_ALT = 0x11,      // release = F0, right = E0
  KB_CAPSLOCK = 0x58, // release = F0,

  KB_UP = 0x75,
  KB_DOWN = 0x72,
  KB_LEFT = 0x6B,
  KB_RIGHT = 0x74,
  KB_HOME = 0x6C,
  KB_END = 0x69,
  KB_PAGE_UP = 0x7D,
  KB_PAGE_DOWN = 0x7A,
  KB_DELETE = 0x71,
  KB_INSERT = 0x70,
  KB_C = 0x21,
};

typedef enum {
  SCAN_STATE_NORMAL, // Expecting  make code, E0, or F0
  SCAN_STATE_E0,     // Received E0, expectin key or F0
  SCAN_STATE_F0,     // Received F0, expecting break code
  SCAN_STATE_E0_F0   // Received E0 then F0, expecting extended break code
} scan_state_t;

static ring_buffer8_t scan_buffer;
static ring_buffer8_t ascii_buffer;
static ring_buffer8_t special_buffer;

static scan_state_t current_state = SCAN_STATE_NORMAL;

static uint8_t modifiers = 0;

static const uint8_t extended_key_map[256] = {
    [KB_UP] = KEY_UP,           [KB_DOWN] = KEY_DOWN,           [KB_LEFT] = KEY_LEFT,
    [KB_RIGHT] = KEY_RIGHT,     [KB_HOME] = KEY_HOME,           [KB_END] = KEY_END,
    [KB_PAGE_UP] = KEY_PAGE_UP, [KB_PAGE_DOWN] = KEY_PAGE_DOWN, [KB_DELETE] = KEY_DELETE,
    [KB_INSERT] = KEY_INSERT,
};

// PRIVATE


// Wait for keyboard controller input buffer to be empty
static void wait_for_ready() {
  for (int i = 0; i < 10000; i++) {
    if (!(port_byte_in(PS2_COMMAND_PORT) & PS2_STATUS_IN_BUF_FULL))
      return;
  }
}

char scan_to_ascii(uint8_t scan_code, uint8_t mods) {
  bool shift_pressed = bit_test(mods, MODIFIER_LSHIFT) || bit_test(mods, MODIFIER_RSHIFT);
  bool caps_on = bit_test(mods, MODIFIER_CAPSLOCK);

  if (shift_pressed != caps_on) {
    return (char)scancodeToAsciiShift[scan_code];
  }

  // TODO: special symbols
  return (char)scancodeToAscii[scan_code];
}

static inline void update_modifier(uint8_t bit, bool is_released) {
  if (is_released)
    bit_clear(&modifiers, bit);
  else
    bit_set(&modifiers, bit);
}

static void handle_keycode(uint8_t scancode, bool is_extended, bool is_released) {
  // clang-format off
  switch (scancode) {
    case KB_LSHIFT: update_modifier(MODIFIER_LSHIFT, is_released); return;
    case KB_RSHIFT: update_modifier(MODIFIER_RSHIFT, is_released); return;
    case KB_CTRL:   update_modifier(is_extended ? MODIFIER_RCTRL : MODIFIER_LCTRL, is_released); return;
    case KB_ALT:    update_modifier(is_extended ? MODIFIER_RALT  : MODIFIER_LALT,  is_released); return;
    case KB_CAPSLOCK:
      if (!is_released) bit_toggle(&modifiers, MODIFIER_CAPSLOCK);
      return;
    default: break;
  }

  // NOTE: dont really care about releases for now
  if (is_released) {
    return;
  }

  if (is_extended) {
    uint8_t mapped_key = extended_key_map[scancode];
    if (mapped_key != 0) {
      rb_push(&special_buffer, mapped_key);
    }
    return;
  }
  // clang-format on

  bool ctrl_pressed = (modifiers & ((1 << MODIFIER_LCTRL) | (1 << MODIFIER_RCTRL))) != 0;
  if (ctrl_pressed && scancode == KB_C) {
    rb_push(&ascii_buffer, 3);
    return;
  }

  // Translate scancode to ascii and push to the ascii buffer
  char ascii = scan_to_ascii(scancode, modifiers);
  if (ascii != 0) {
    rb_push(&ascii_buffer, ascii);
  }
}

void process_byte() {
  uint8_t byte = 0;
  if (!rb_pop(&scan_buffer, &byte)) {
    return;
  }

  switch (current_state) {
    case SCAN_STATE_NORMAL:
      if (byte == 0xE0)
        current_state = SCAN_STATE_E0;
      else if (byte == 0xF0)
        current_state = SCAN_STATE_F0;
      else {
        handle_keycode(byte, false, false);
        current_state = SCAN_STATE_NORMAL;
      }
      break;

    case SCAN_STATE_E0:
      if (byte == 0xF0)
        current_state = SCAN_STATE_E0_F0;
      else {
        handle_keycode(byte, true, false);
        current_state = SCAN_STATE_NORMAL;
      }
      break;
    case SCAN_STATE_F0: // released
      handle_keycode(byte, false, true);
      current_state = SCAN_STATE_NORMAL;
      break;

    case SCAN_STATE_E0_F0:
      // extended key released
      handle_keycode(byte, true, true);
      current_state = SCAN_STATE_NORMAL;
      break;
  }
}

// INTERRUPT HANDLER

void keyboard_handler(registers_t* regs) {
  uint8_t scan_code = port_byte_in(0x60);
  rb_push(&scan_buffer, scan_code);
}

// PUBLIC

void init_keyboard() {
  log_info("Initialising PS/2 Keyboard...\n");
  rb_init(&scan_buffer);
  rb_init(&ascii_buffer);
  rb_init(&special_buffer);

  register_interrupt_handler(IRQ1, keyboard_handler);
  set_keymap("us");
}

void keyboard_set_scancode_set2() {
  log_debug("Setting scancode set 2\n");
  wait_for_ready();
  port_byte_out(PS2_COMMAND_PORT, PS2_CMD_READ_CONFIG);
  uint8_t config = port_byte_in(PS2_DATA_PORT);
  config &= ~(1 << 6);

  wait_for_ready();
  port_byte_out(PS2_COMMAND_PORT, PS2_CMD_WRITE_CONFIG);
  port_byte_out(PS2_DATA_PORT, config);
  wait_for_ready();

  port_byte_out(PS2_DATA_PORT, KB_CMD_SET_SCANCODE_SET);
  wait_for_ready();
  port_byte_out(PS2_DATA_PORT, 0x02); // scancode set 2

  uint8_t response = port_byte_in(PS2_DATA_PORT);
  if (response != 0xFA) // ACK
    fprintf(stderr, "Received: %x", response);
}

void decode_keyboard() {
  while (!rb_is_empty(&scan_buffer)) {
    process_byte();
  }
}

bool pop_key(uint8_t* out) {
  return rb_pop(&ascii_buffer, out);
}


bool pop_special_key(uint8_t* out) {
  return rb_pop(&special_buffer, out);
}


bool keyboard_has_key(void) {
  return !rb_is_empty(&ascii_buffer);
}

bool has_special_key() {
  return !rb_is_empty(&special_buffer);
}
