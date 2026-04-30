void kernel_main(unsigned long magic, unsigned long multiboot_info)
{
    (void)magic;
    (void)multiboot_info;

    volatile unsigned short *video_memory = (volatile unsigned short *)0xB8000;
    const char *message = "booted yay";

    for (unsigned int i = 0; message[i] != '\0'; i++) {
        video_memory[i] = (unsigned short)message[i] | 0x0F00;
    }

    for (;;) {
        __asm__ volatile ("hlt");
    }
}
