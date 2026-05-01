#ifndef KERNEL_CLI_H
#define KERNEL_CLI_H

#ifdef __cplusplus
extern "C" {
#endif

// Prints the terminal prompt
void cli_print_prompt(void);

// Runs one full command line
void cli_submit_line(const char *line);

#ifdef __cplusplus
}
#endif

#endif
