#include <stdio.h>
#include <string.h>

#include "shell/shell_command.h"

/**
 * Displays all available shell commands with aligned descriptions.
 * Finds the longest command name and uses it to pad all other command
 * names so descriptions line up nicely in two columns.
 * @param argc Number of arguments (unused)
 * @param argv Argument list (unused)
 * @return 0 on success
 */
int cmd_help(int argc, char** argv) {
  (void)argc;
  (void)argv;

  // Find the longest command name to determine column width
  size_t max_len = 0;
  for (size_t i = 0; i < NUM_COMMANDS; i++) {
    size_t len = strlen(command_table[i].name);
    if (len > max_len) {
      max_len = len;
    }
  }

  // Print each command with padded name and description
  for (size_t i = 0; i < NUM_COMMANDS; i++) {
    // %-*s prints the string left-aligned with minimum width
    printf("%-*s  %s\n", (int)max_len, command_table[i].name, command_table[i].description);
  }
  return 0;
}
