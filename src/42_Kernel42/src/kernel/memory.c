#include "kernel/memory.h"
#include "kernel/pmm.h"
#include <kernel/log.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

void print_memory_layout() {
    printf("Memory Layout:\n");
    printf("  Kernel start: 0x100000\n");
    printf("  PMM Bitmap:    0x500000\n");
    printf("  Page Dir:      0x104000\n");
    printf("  Page Tables:   0x108000\n");
    printf("  User Space:     0x08000000\n");
    printf("\n");
    printf("Physical Memory:\n");
    printf("  Total frames:  %d\n", pmm_get_total_count());
    printf("  Free frames:   %d\n", pmm_get_free_count());
    printf("  Used frames:   %d\n", pmm_get_total_count() - pmm_get_free_count());
    printf("  Free memory:   %d KB\n", pmm_get_free_count() * 4);
}
