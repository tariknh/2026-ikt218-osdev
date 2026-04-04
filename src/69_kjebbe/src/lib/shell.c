#include "../../include/keyboard.h"
#include "../../include/libc/stdio.h"
void shell_keyboard_handler(int scancode) {
  int key = get_key(scancode);
  if (key != -1) {
    printf("%c", key);
  }
}
