#include "kernel/memory.h"
#include "stdio.h"

void *operator new(size_t size) {
    return malloc(size);
}

void *operator new[](size_t size) {
    return malloc(size);
}

void operator delete(void *ptr) noexcept {
    free(ptr);
}

void operator delete[](void *ptr) noexcept {
    free(ptr);
}

void operator delete(void *ptr, size_t) noexcept {
    free(ptr);
}

void operator delete[](void *ptr, size_t) noexcept {
    free(ptr);
}

extern "C" void test_new_operator(void) {
    int *value = new int(13);
    printf("new operator: 0x%x = %d\n", value, *value);
    delete value;
}
