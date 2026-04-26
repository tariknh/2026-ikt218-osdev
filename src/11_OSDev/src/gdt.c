#include "gdt.h"


static struct gdt_entry      gdt[3];
static struct gdt_descriptor gdt_ptr;


extern void gdt_flush(uint32_t gdt_ptr_addr);


static void gdt_set_entry(int num, uint32_t base, uint32_t limit,
                          uint8_t access, uint8_t gran)
{
    gdt[num].base_low    = (uint16_t)(base  & 0xFFFF);
    gdt[num].base_middle = (uint8_t) ((base  >> 16) & 0xFF);
    gdt[num].base_high   = (uint8_t) ((base  >> 24) & 0xFF);

    gdt[num].limit_low   = (uint16_t)(limit & 0xFFFF);

    
    gdt[num].granularity = (uint8_t)(((limit >> 16) & 0x0F) | (gran & 0xF0));

    gdt[num].access = access;
}


void gdt_init(void)
{
    gdt_ptr.limit = (uint16_t)((sizeof(struct gdt_entry) * 3) - 1);
    gdt_ptr.base  = (uint32_t)&gdt;

    
    gdt_set_entry(0, 0, 0, 0x00, 0x00);

    
    gdt_set_entry(1, 0x00000000, 0xFFFFFFFF, 0x9A, 0xCF);

    
    gdt_set_entry(2, 0x00000000, 0xFFFFFFFF, 0x92, 0xCF);

    
    gdt_flush((uint32_t)&gdt_ptr);
}
