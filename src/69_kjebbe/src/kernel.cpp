#include "../include/libc/stddef.h" for size_t
#include "../include/libc/stdint.h" for size_t

// Malloc and free from the given C implementation
extern "C" void *malloc(size_t size);
extern "C" void free(void *ptr);

//  C++ new and delete operator use our malloc and free
void *operator new(size_t size) { return malloc(size); }
void *operator new[](size_t size) { return malloc(size); }
void operator delete(void *ptr) noexcept { free(ptr); }
void operator delete[](void *ptr) noexcept { free(ptr); }

extern "C" int kernel_main(void);
int kernel_main() { return 0; }
