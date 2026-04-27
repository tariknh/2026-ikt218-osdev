#include <kernel/memory.h>

int cmd_print_memory(int argc, char** argv) {
  (void)argc;
  (void)(argv);
  print_memory_layout();
  return 0;
}
