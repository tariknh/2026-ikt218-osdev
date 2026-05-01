# Kernel Source Overview

This directory contains the core source files for the 32-bit kernel.

## Boot And Execution Flow

1. The bootloader transfers control to `_start` in `multiboot2.asm`.
2. `_start` sets up a stack and calls `main` in `kernel.c`.
3. `main` initializes the Global Descriptor Table (GDT).
4. After the GDT is loaded and segment registers are reloaded, the kernel initializes the VGA text terminal.
5. The kernel prints `Hello, World!` and then halts in an infinite loop.

## GDT Files

- `gdt.c` creates the three required GDT entries:
  - null descriptor
  - kernel code descriptor
  - kernel data descriptor
- `arch/i386/gdt_flush.asm` loads the GDT with `lgdt` and reloads the segment registers.

## Terminal Output

`terminal.c` writes directly to VGA text memory at `0xB8000`, which is why the kernel can print text without a standard library or drivers.