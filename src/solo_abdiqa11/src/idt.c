#include "idt.h"

// IDT table + pointer
static idt_entry_t idt[IDT_ENTRIES];
idt_ptr_t idtp;


// Implement your own memset to avoid libc dependency
static void *memset_local(void *dst, int v, unsigned int n) {
    unsigned char *p = (unsigned char *)dst;
    while (n--) *p++ = (unsigned char)v;
    return dst;
}

// Assembly function that loads IDTR
extern void idt_load(idt_ptr_t *ptr);



void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low  = (uint16_t)(base & 0xFFFF);
    idt[num].base_high = (uint16_t)((base >> 16) & 0xFFFF);

    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

void idt_init(void) {
    // Build the pointer
    idtp.limit = (uint16_t)(sizeof(idt_entry_t) * IDT_ENTRIES - 1);
    idtp.base  = (uint32_t)&idt[0];

    // Clear the whole IDT (so everything is "not present" for now)
    memset_local(idt, 0, sizeof(idt));

    // Load IDTR (does NOT enable interrupts)
    idt_load(&idtp);
}
