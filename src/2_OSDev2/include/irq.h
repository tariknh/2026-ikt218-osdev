#pragma once
#include <libc/stdint.h>
#include "isr.h"

void irq_init(void); // Initializes the IRQs and remaps the PIC
void irq_handler(isr_frame_t* frame); // Common handler for all IRQs