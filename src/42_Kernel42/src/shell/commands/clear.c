#include "drivers/video/vga_text.h"

int cmd_clear(int argc, char** argv) {
  (void)argc;
  (void)argv;
  vga_text_clear_screen();
  return 0;
}
