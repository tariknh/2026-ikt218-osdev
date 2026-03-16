#ifndef PIC_H
#define PIC_H

#include <libc/stdint.h>

void pic_remap(void);
void pic_mask_all(void);

static inline void outb(uint16_t port, uint8_t value)
{
	__asm__ __volatile__("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline void io_wait(void)
{
	__asm__ __volatile__("outb %%al, $0x80" : : "a"(0));
}

#endif // PIC_H