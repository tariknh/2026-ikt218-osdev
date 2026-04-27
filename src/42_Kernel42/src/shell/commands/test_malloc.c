#include <stdio.h>
#include <kernel/memory.h>
#include <kernel/pmm.h>
#include <libc/string.h>

extern memory_info_t memory_info;

int cmd_test_malloc(int argc, char** argv) {
    (void)argc;
    (void)argv;

    printf("=== Testing malloc with alloc_t ===\n\n");

    uint32_t free_before = pmm_get_free_count();
    printf("Before allocation:\n");
    printf("  Free frames: %d\n", free_before);
    printf("  Initial memory used: %d bytes\n\n", memory_info.memory_used);
    uint32_t mem_start = memory_info.memory_used;

    printf("Allocating 100 bytes...\n");
    char* ptr1 = malloc(100);
    if (!ptr1) {
        printf("ERROR: malloc(100) returned NULL!\n");
        return -1;
    }
    printf("  Allocated at 0x%x\n", (uint32_t)ptr1);

    printf("Allocating 50 bytes...\n");
    char* ptr2 = malloc(50);
    if (!ptr2) {
        printf("ERROR: malloc(50) returned NULL!\n");
        return -1;
    }
    printf("  Allocated at 0x%x\n", (uint32_t)ptr2);

    printf("Allocating 5000 bytes (spans pages)...\n");
    char* ptr3 = malloc(5000);
    if (!ptr3) {
        printf("ERROR: malloc(5000) returned NULL!\n");
        return -1;
    }
    printf("  Allocated at 0x%x\n", (uint32_t)ptr3);

    uint32_t free_after_alloc = pmm_get_free_count();
    printf("\nAfter 3 allocations:\n");
    printf("  Free frames: %d (used %d)\n", free_after_alloc, free_before - free_after_alloc);
    printf("  Memory used: %d bytes\n", memory_info.memory_used);

    printf("\nWriting to allocations...\n");
    memcpy(ptr1, "Hello from malloc 100", 21);
    memcpy(ptr2, "malloc 50 ok", 12);
    memcpy(ptr3, "This is 5000 bytes of data for testing malloc", 49);
    printf("  ptr1: \"%s\"\n", ptr1);
    printf("  ptr2: \"%s\"\n", ptr2);
    printf("  ptr3: \"%s\"\n", ptr3);

    printf("\nFreeing all allocations...\n");
    free(ptr1);
    free(ptr2);
    free(ptr3);

    uint32_t free_after_free = pmm_get_free_count();
    printf("\nAfter freeing all:\n");
    printf("  Free frames: %d\n", free_after_free);
    printf("  Memory used: %d bytes\n", memory_info.memory_used);

    printf("\n=== Verification ===\n");
    int pass = 1;

    if (free_after_free == free_before) {
        printf("  [PASS] Frame count restored (%d == %d)\n", free_after_free, free_before);
    } else {
        printf("  [FAIL] Expected %d frames, got %d\n", free_before, free_after_free);
        pass = 0;
    }

    if (memory_info.memory_used == mem_start) {
        printf("  [PASS] Memory used restored to initial value (%d)\n", mem_start);
    } else {
        printf("  [FAIL] Expected %d bytes, got %d\n", mem_start, memory_info.memory_used);
        pass = 0;
    }

    printf("\n=== Test %s ===\n", pass ? "PASSED" : "FAILED");
    return pass ? 0 : -1;
}