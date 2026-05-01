#ifndef KERNEL_IDT_H
#define KERNEL_IDT_H

#include <libc/stdint.h>

#define IDT_ENTRIES 256

struct IdtEntry{
	uint16_t    interrupt_low; // lower 16 bits of interrupt handler's address
	uint16_t    kernel_cs; // tells the CPU which code segment to use when handling interrupt
	uint8_t     reserved; // unused, but must exist
	uint8_t     attributes; // defines how interrupt behaves
	uint16_t    interrupt_high; // higher 16 bits of interrupt handler's address
} __attribute__((packed));

// Interrupt descriptor table register
struct Idtr {
    uint16_t    limit;
    uint32_t    base;
} __attribute__((packed));

void IdtInitialize(void);
void IdtSetDescriptor(uint8_t vector, uint32_t interrupt, uint8_t flags);

#endif