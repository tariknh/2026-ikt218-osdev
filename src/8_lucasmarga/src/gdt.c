#include "gdt.h"
#include "libc/stdint.h"

typedef struct __attribute__((packed)) {
    uint16_t limit_low;      
    uint16_t base_low;      
    uint8_t  base_mid;       
    uint8_t  access;         
    uint8_t  granularity;    
    uint8_t  base_high;      
} gdt_descriptor;


typedef struct __attribute__((packed)) {
    uint16_t limit;
    uint32_t base;
} gdt_register;

static gdt_descriptor gdt[3];
static gdt_register   gdtr;


extern void gdt_load(const gdt_register* gdtr);


static void gdt_create_entry(int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    gdt[index].limit_low = (uint16_t)(limit & 0xFFFF);
    gdt[index].base_low  = (uint16_t)(base & 0xFFFF);
    gdt[index].base_mid  = (uint8_t)((base >> 16) & 0xFF);
    gdt[index].access    = access;

    gdt[index].granularity = (uint8_t)((limit >> 16) & 0x0F);

    gdt[index].granularity |= (uint8_t)(flags & 0xF0);

    gdt[index].base_high = (uint8_t)((base >> 24) & 0xFF);
}

void gdt_setup(void) {
    
    gdt_create_entry(0, 0, 0, 0, 0);

   
    gdt_create_entry(1, 0x00000000, 0x000FFFFF, 0x9A, 0xC0); 
    gdt_create_entry(2, 0x00000000, 0x000FFFFF, 0x92, 0xC0); 

    gdtr.limit = (uint16_t)(sizeof(gdt) - 1);
    gdtr.base  = (uint32_t)&gdt[0];

    gdt_load(&gdtr);
}