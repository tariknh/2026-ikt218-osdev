#ifndef PIC_H
#define PIC_H

#include <libc/stdint.h>

void pic_remap(void);
void pic_send_eoi(uint8_t irq);

#endif