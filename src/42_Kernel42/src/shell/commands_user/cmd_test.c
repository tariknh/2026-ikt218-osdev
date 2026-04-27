#include <stdint.h>

#define SYS_write 4
#define SYS_exit  1

static inline uint32_t syscall(uint32_t n, uint32_t a, uint32_t b, uint32_t c) {
    uint32_t r;
    __asm__ volatile(
        "movl %1, %%eax; movl %2, %%ebx; movl %3, %%ecx; movl %4, %%edx; int $0x80; movl %%eax, %0"
        : "=g"(r) : "g"(n), "g"(a), "g"(b), "g"(c) : "eax","ebx","ecx","edx");
    return r;
}

void write_str(const char* s) {
    int len = 0;
    while (s[len]) len++;
    syscall(SYS_write, 1, (uint32_t)s, len);
}

void _start(void) {
    write_str("Test command running!\n");
    while(1) { __asm__ volatile("hlt"); }
}