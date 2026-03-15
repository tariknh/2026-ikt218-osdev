__attribute__((noreturn))
void exception_handler(void);
void exception_handler() {
    while (1)
        __asm__ volatile ("cli; hlt");
}