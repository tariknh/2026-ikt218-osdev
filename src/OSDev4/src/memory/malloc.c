#include "memory.h"
#include <stdint.h>

extern void terminal_write(const char* str);

static uint8_t* heap_begin = 0;
static uint8_t* heap_end = 0;
static uint32_t memory_used = 0;

void init_kernel_memory(uint32_t* kernel_end)
{
    heap_begin = (uint8_t*)kernel_end;
    heap_end = heap_begin + 0x100000;
    memory_used = 0;
}

void* malloc(uint32_t size)
{
    uint8_t* current = heap_begin;

    while (current < heap_end) {
        alloc_t* header = (alloc_t*)current;

        if (header->size == 0) {
            header->status = 1;
            header->size = size;
            memory_used += size + sizeof(alloc_t);
            return (void*)(current + sizeof(alloc_t));
        }

        if (header->status == 0 && header->size >= size) {
            header->status = 1;
            memory_used += size;
            return (void*)(current + sizeof(alloc_t));
        }

        current += sizeof(alloc_t) + header->size;
    }

    return (void*)0;
}

void* pmalloc(uint32_t size)
{
    return malloc(size);
}

void free(void* ptr)
{
    alloc_t* header = (alloc_t*)((uint8_t*)ptr - sizeof(alloc_t));
    header->status = 0;
    memory_used -= header->size;
}

static void print_hex(uint32_t val)
{
    char buf[11];
    buf[0] = '0';
    buf[1] = 'x';
    buf[10] = '\0';
    int i;
    for (i = 9; i >= 2; i--) {
        uint8_t nibble = val & 0xF;
        buf[i] = nibble < 10 ? '0' + nibble : 'A' + (nibble - 10);
        val >>= 4;
    }
    terminal_write(buf);
}

static void print_dec(uint32_t val)
{
    char buf[11];
    int i = 10;
    buf[10] = '\0';
    if (val == 0) {
        terminal_write("0");
        return;
    }
    while (val > 0 && i > 0) {
        buf[--i] = '0' + (val % 10);
        val /= 10;
    }
    terminal_write(&buf[i]);
}

void print_memory_layout(void)
{
    terminal_write("Heap begin: ");
    print_hex((uint32_t)heap_begin);
    terminal_write("\n");
    terminal_write("Heap end: ");
    print_hex((uint32_t)heap_end);
    terminal_write("\n");
    terminal_write("Memory used: ");
    print_dec(memory_used);
    terminal_write(" bytes\n");
}
