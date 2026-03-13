#ifndef ISR_H
#define ISR_H

#include <stdint.h>

void isr_install(void);
void isr_handler(uint32_t int_no);

#endif
