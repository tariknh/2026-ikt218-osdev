#include "kernel/memory.h"
#include <libc/stdint.h>

void* operator new(unsigned int size) {
    if (size == 0) {
        size = 1;
    }
    return malloc(size);
}

void* operator new[](unsigned int size) {
    if (size == 0) {
        size = 1;
    }
    return malloc(size);
}

void operator delete(void* ptr) noexcept {
    if (ptr) {
        free(ptr);
    }
}

void operator delete[](void* ptr) noexcept {
    if (ptr) {
        free(ptr);
    }
}

void operator delete(void* ptr, unsigned int) noexcept {
    if (ptr) {
        free(ptr);
    }
}

void operator delete[](void* ptr, unsigned int) noexcept {
    if (ptr) {
        free(ptr);
    }
}

extern "C" uint32_t cpp_new_test(void) {
    int* value = new int(42);
    if (!value) {
        return 0;
    }

    uint32_t result = (uint32_t)(*value);
    delete value;
    return result;
}
