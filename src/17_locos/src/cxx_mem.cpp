#include <libc/stddef.h>
#include <libc/stdint.h>

extern "C" {
#include "memory.h"
#include "terminal.h"
}

void* operator new(size_t size) {
    return malloc((uint32_t)size);
}

void* operator new[](size_t size) {
    return malloc((uint32_t)size);
}

void operator delete(void* ptr) noexcept {
    free(ptr);
}

void operator delete[](void* ptr) noexcept {
    free(ptr);
}

void operator delete(void* ptr, size_t) noexcept {
    free(ptr);
}

void operator delete[](void* ptr, size_t) noexcept {
    free(ptr);
}

struct test_object {
    uint32_t a;
    uint32_t b;
    uint32_t c;
};

extern "C" bool cpp_new_delete_test(void) {
    test_object* obj = new test_object;
    if (!obj) {
        terminal_printf("C++ new failed\n");
        return false;
    }

    obj->a = 0x2B10;
    obj->b = 0xB105;
    obj->c = obj->a ^ obj->b;

    terminal_printf("C++ new ptr=0x%x values: a=0x%x b=0x%x c=0x%x\n",
                    (uint32_t)(uintptr_t)obj, obj->a, obj->b, obj->c);

    bool ok = (obj->c == (0x2B10u ^ 0xB105u));
    delete obj;
    return ok;
}
