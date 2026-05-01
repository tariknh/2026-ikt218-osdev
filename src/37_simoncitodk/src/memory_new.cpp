#include "memory.h"

void *operator new(__SIZE_TYPE__ size)
{
    return malloc(size);
}

void *operator new[](__SIZE_TYPE__ size)
{
    return malloc(size);
}

void operator delete(void *ptr) noexcept
{
    free(ptr);
}

void operator delete[](void *ptr) noexcept
{
    free(ptr);
}

void operator delete(void *ptr, __SIZE_TYPE__ size) noexcept
{
    (void)size;
    free(ptr);
}

void operator delete[](void *ptr, __SIZE_TYPE__ size) noexcept
{
    (void)size;
    free(ptr);
}

extern "C" uint32_t *test_new_operator(void)
{
    uint32_t *value = new uint32_t;
    *value = 1234;
    return value;
}
