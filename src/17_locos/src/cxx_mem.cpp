#include <libc/stddef.h>
#include <libc/stdint.h>

extern "C" {
#include "memory.h"
#include "terminal.h"
}

// Route C++ allocations to the kernel allocator
void* operator new(size_t size) {
    return malloc((uint32_t)size);
}

// Route C++ arrays to the kernel allocator
void* operator new[](size_t size) {
    return malloc((uint32_t)size);
}

// Route delete to the kernel free function
void operator delete(void* ptr) noexcept {
    free(ptr);
}

// Route delete[] to the kernel free function
void operator delete[](void* ptr) noexcept {
    free(ptr);
}

// Sized delete uses the same free path
void operator delete(void* ptr, size_t) noexcept {
    free(ptr);
}

// Sized delete[] uses the same free path
void operator delete[](void* ptr, size_t) noexcept {
    free(ptr);
}

// Small object used for the C++ test
struct test_object {
    uint32_t a;
    uint32_t b;
    uint32_t c;
};

// Check that new and delete work in the kernel
extern "C" bool cpp_new_delete_test(void) {
    // Allocate one object through the kernel heap
    test_object* obj = new test_object;
    if (!obj) {
        terminal_printf("C++ new failed\n");
        return false;
    }

    // Fill the object with simple test values
    obj->a = 0x2B10;
    obj->b = 0xB105;
    obj->c = obj->a ^ obj->b;

    // Print the pointer and the values
    terminal_printf("C++ new ptr=0x%x values: a=0x%x b=0x%x c=0x%x\n",
                    (uint32_t)(uintptr_t)obj, obj->a, obj->b, obj->c);

    // Check that the math result is correct
    bool ok = (obj->c == (0x2B10u ^ 0xB105u));
    delete obj;
    return ok;
}
