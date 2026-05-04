#pragma once
#include <libc/stdint.h>

void keyboard_init(void); // Initializes the keyboard driver and sets up the IRQ handler
void keyboard_on_irq1(void); // Called when IRQ1 (keyboard interrupt) is triggered