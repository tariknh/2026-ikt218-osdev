Finish Task 2 first (3 visible software ISR messages)
Add 3 test interrupts in kernel after idt_init():
int $0x0, int $0x3, int $0x6
Make ISR handler print which vector fired.
Stop once this works.
Then Task 3 (IRQ0-15 support)
Implement PIC remap.
Add IRQ stubs mapped to vectors 32-47.
In IRQ handler send EOI properly.
Then Task 4 (keyboard logger)
Handle IRQ1.
Read scancode from port 0x60.
Ignore key-release scancodes (high bit set).
Translate via lookup table.
Print char + append to buffer.