#include <stdint.h>
#include <stddef.h>
#include "memory.h"

// Current placement pointer for the bump allocator
static uint32_t placement_address = 0;

void init_kernel_memory(uint32_t* kernel_end) {
    // The heap starts right after the kernel image
    placement_address = (uint32_t)kernel_end;
}

void* malloc(uint32_t size) {
    // Align to 4 bytes
    if (placement_address & 0x3) {
        placement_address = (placement_address & ~0x3) + 4;
    }

    uint32_t addr = placement_address;
    placement_address += size;
    return (void*)addr;
}

void free(void* ptr) {
    // Bump allocator: no-op
    (void)ptr;
}

void print_memory_layout() {
    // We print hex addresses manually since printf is minimal
    extern void terminal_writestring(const char*);

    // Helper to print a 32-bit hex value
    char buf[11]; // "0x" + 8 hex digits + null
    buf[0] = '0'; buf[1] = 'x';
    uint32_t val = placement_address;
    for (int i = 9; i >= 2; i--) {
        int nibble = val & 0xF;
        buf[i] = (nibble < 10) ? ('0' + nibble) : ('A' + nibble - 10);
        val >>= 4;
    }
    buf[10] = '\0';

    terminal_writestring("[Memory] Heap placement pointer: ");
    terminal_writestring(buf);
    terminal_writestring("\n");

    // Also print kernel end
    val = (uint32_t)&end;
    for (int i = 9; i >= 2; i--) {
        int nibble = val & 0xF;
        buf[i] = (nibble < 10) ? ('0' + nibble) : ('A' + nibble - 10);
        val >>= 4;
    }
    terminal_writestring("[Memory] Kernel end address:     ");
    terminal_writestring(buf);
    terminal_writestring("\n");
}
