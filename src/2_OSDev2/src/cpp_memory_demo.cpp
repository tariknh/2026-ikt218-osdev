#include <libc/stdint.h>

extern "C" void* cpp_allocate_demo(void) {
    uint32_t* numbers = new uint32_t[64];
    if (numbers != nullptr) {
        for (uint32_t i = 0; i < 64; i++) {
            numbers[i] = i;
        }
    }
    return numbers;
}

extern "C" void cpp_free_demo(void* ptr) {
    delete[] static_cast<uint32_t*>(ptr);
}