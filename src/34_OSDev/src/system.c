#include <libc/system.h>
#include <libc/stdio.h>

void panic(const char* panic_message) {
    printf("PANIC: %s\n", panic_message);

    while(1) {}
}