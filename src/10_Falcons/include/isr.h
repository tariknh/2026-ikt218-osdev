#ifndef ISR_H
#define ISR_H

#include <libc/stdint.h>

void isr_install(void);
void isr_handler(uint32_t interrupt_number);

#endif