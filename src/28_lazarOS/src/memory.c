#include <memory.h>

static uint32_t* heap_ptr = 0;

void init_kernel_memory(uint32_t* kernel_end){
    heap_ptr = kernel_end;
}

void* malloc(size_t size){
    void *addr = (void*)heap_ptr;
    heap_ptr = (uint32_t*)((uint8_t*)heap_ptr + size);
    return addr;
}

void free(void* ptr) {
}

void init_paging() {
    // will implement later
}

void print_memory_layout() {
    // will implement later
}