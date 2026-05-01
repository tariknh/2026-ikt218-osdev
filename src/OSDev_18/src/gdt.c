#include <kernel/gdt.h>

static struct GdtEntry gdtEntries[3];
static struct GdtDescriptor gdtDescriptor;

extern void GdtFlush(uint32_t gdtDescriptorAddress);

static void GdtSetEntry(
    uint32_t index,
    uint32_t base,
    uint32_t limit,
    uint8_t  access,
    uint8_t  granularity
) {
    // x86 stores the descriptor fields split across several smaller fields.
    gdtEntries[index].base_low = (uint16_t)(base & 0xFFFF);
    gdtEntries[index].base_middle = (uint8_t)((base >> 16) & 0xFF);
    gdtEntries[index].base_high = (uint8_t)((base >> 24) & 0xFF);

    gdtEntries[index].limit_low = (uint16_t)(limit & 0xFFFF);
    gdtEntries[index].granularity = (uint8_t)((limit >> 16) & 0x0F);
    gdtEntries[index].granularity |= (uint8_t)(granularity & 0xF0);
    gdtEntries[index].access = access;
}

void GdtInitialize(void) {
    gdtDescriptor.size = sizeof(gdtEntries) - 1;
    gdtDescriptor.offset = (uint32_t)&gdtEntries;

    GdtSetEntry(0, 0, 0, 0, 0);

    // Flat kernel code segment covering the full 32-bit address space.
    GdtSetEntry(1, 0, 0x000FFFFF, 0x9A, 0xCF);

    // Matching flat data segment used by ds/es/fs/gs/ss after the flush.
    GdtSetEntry(2, 0, 0x000FFFFF, 0x92, 0xCF);

    GdtFlush((uint32_t)&gdtDescriptor);
}
