#pragma once

/**
 * Initialize the shell, clearing the buffer, and disabling the hardware cursor
 */
void shell_init(void);

/**
 * Run the shell application, calls shell_init
 */
void shell_run(void);

/**
 * Get current working directory
 * @return pointer to cwd string
 */
const char* shell_get_cwd(void);

/**
 * Set current working directory
 * @param new_path string path
 */
void shell_set_cwd(const char* new_path);