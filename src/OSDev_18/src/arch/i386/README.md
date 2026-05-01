# i386 Architecture Notes

This kernel runs in 32-bit protected mode on i386.

## Global Descriptor Table

The kernel installs a minimal Global Descriptor Table (GDT) with three entries:

1. A null descriptor
2. A kernel code descriptor
3. A kernel data descriptor

The code and data descriptors both use:

- base address `0x00000000`
- effective 4 GiB address space
- 4 KiB granularity
- ring 0 privilege level

## Why `lgdt` Is Not Enough

`lgdt` only loads the processor's GDT register. It does not automatically refresh the segment registers that are already cached by the CPU.

Because of that, the kernel must:

1. Execute `lgdt`
2. Perform a far jump to reload `cs`
3. Reload `ds`, `es`, `fs`, `gs`, and `ss` with the new data selector

That reload sequence is implemented in `gdt_flush.asm`.