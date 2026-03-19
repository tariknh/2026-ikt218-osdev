#include "../include/libc/stdint.h"
#include "../include/libc/stddef.h"
#include "../include/libc/string.h"
#include "../include/gdt.h"

int main(uint32_t magic, uint32_t mb_info_addr) {
    gdt_init();

    char* video_memory = (char*)0xb8000;
    
    char* line1 = "GDT Initialized!";
    char* line2 = "Hello, World!";
    
    // Print line 1
    for (size_t i = 0; line1[i] != '\0'; i++) {
        video_memory[i * 2] = line1[i];
        video_memory[i * 2 + 1] = 0x07;
    }
    
    // Print line 2 on second row (row 1 = 80 characters offset)
    for (size_t i = 0; line2[i] != '\0'; i++) {
        video_memory[(80 + i) * 2] = line2[i];
        video_memory[(80 + i) * 2 + 1] = 0x07;
    }

    while (1) __asm__ volatile("hlt");
    return 0;
}