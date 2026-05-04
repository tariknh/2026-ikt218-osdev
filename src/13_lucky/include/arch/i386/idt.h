#pragma once

#include "stdint.h"

void init_idt(void);

void idt_set_gate(uint8_t index, uint32_t base, uint8_t attributes);

void enable_interrupts(void);

void disable_interrupts(void);
