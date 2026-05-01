#include "libc/system.h"

__attribute__((noreturn))
void panic(const char* reason)
{
    printf("\n\n!!! PANIC !!!\n%s\n", reason);
    while (1) asm volatile("cli; hlt");
}
