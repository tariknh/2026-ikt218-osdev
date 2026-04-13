extern "C" void isr_initialize(void);
extern "C" void irq_initialize(void);
extern "C" int kernel_main(void);

int kernel_main() {
    isr_initialize();
	irq_initialize();
	asm volatile("sti");

    while (1) {
		asm volatile("hlt");
    }

    return 0;
}