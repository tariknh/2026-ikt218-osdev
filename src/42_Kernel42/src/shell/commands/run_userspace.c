#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <kernel/user_mode.h>
#include <kernel/paging.h>
#include <kernel/pmm.h>

extern const uint8_t shell_elf[];
extern const uint32_t shell_elf_size;

int run_userspace_handler(int argc, char** argv) {
    (void)argc;
    (void)argv;

    printf("Reloading shell ELF (size %d bytes)\n", shell_elf_size);

    elf_info_t info;
    if (elf_get_info(shell_elf, &info) < 0) {
        printf("Failed to parse ELF\n");
        return -1;
    }

    printf("ELF entry: 0x%x\n", info.entry);

    if (elf_load(shell_elf) < 0) {
        printf("Failed to load ELF\n");
        return -1;
    }

    uint32_t stack_top = 0x08040000;
    uint32_t stack_pages = 4;
    printf("Setting up stack at 0x%x\n", stack_top);
    for (uint32_t page = stack_top - (stack_pages * 4096); page < stack_top; page += 4096) {
        uint32_t phys = pmm_alloc_frame();
        if (!phys) {
            printf("Failed to allocate stack frame\n");
            return -1;
        }
        memset((void*)phys, 0, 4096);
        vmm_map_user_page(page, phys, PAGE_USER_RW);
    }

    printf("About to call switch_to_user_mode(0x%x, 0x%x)\n", info.entry, stack_top);
    __asm__ volatile("cli");  // Disable interrupts before switch
    switch_to_user_mode(info.entry, stack_top);
    printf("ERROR: Returned from user mode!\n");

    return 0;
}