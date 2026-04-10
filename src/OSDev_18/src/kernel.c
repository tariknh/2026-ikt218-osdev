#include <libc/stdint.h>
#include <kernel/terminal.h>

void main(void) {
    terminalInitialize();
    terminalWriteString("Hello, World!\n");
    
    for (;;) {
        __asm__ volatile ("hlt");
    }
}