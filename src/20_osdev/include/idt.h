#ifndef IDT_H
#define IDT_H

#include <libc/stdint.h>

void idt_init(void);
void idt_set_gate(int index, uint32_t handler);

#endif