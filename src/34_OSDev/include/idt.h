#ifndef IDT_H
#define IDT_H

#include <libc/stdint.h>

struct idt_entry {
    uint16_t offset_low;  //bit 0-15 of the handler functions address
    uint16_t selector;    //code segment selector (kernel code = 0x08)
    uint8_t  zero;        //always 0, reserved by CPU
    uint8_t  type_attr;   //present bit, privilege level and gate type
    uint16_t offset_high; //bit 16-31 of the handler functions address
} __attribute__((packed)); //no padding cause CPU expects bytes in exact positions

//IDT pointer is passed to lidt which tells CPU location and size of IDT
struct idt_ptr {
    uint16_t limit; //size of IDT (bytes-1)
    uint32_t base;  //memory address of IDT
} __attribute__((packed));

void init_idt(void);

#endif