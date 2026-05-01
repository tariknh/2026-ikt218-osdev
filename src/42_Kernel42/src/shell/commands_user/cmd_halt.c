#include <stdint.h>

void _start(void) {
    /* Just halt - no syscalls at all */
    while(1) { __asm__ volatile("hlt"); }
}