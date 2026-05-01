; Name: gdt.asm
; Project: LocOS
; Description: This file contains the assembly code to load the Global Descriptor Table (GDT) into the CPU.

global gdt_load ; Exports the symbol gdt_load so that C code can call it

section .text ; Places the following code in executable text section
bits 32 ; Tells NASM this is 32 bit code

gdt_load: ;Defines the function entry point for loading the GDT
    ; argument: pointer to gdt_ptr_t is on stack
    mov eax, [esp + 4] ; Loads the first function argument from the stack and into eax register
    lgdt [eax] ; Loads the CPUs GDTR ( A special register will say more in overleaf) from the memory pointed to by eax

    ; Reload data segment registers with selector 0x10
    ; selector 0x10 = entry 2 (data segment), index 2 * 8
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Far jump to reload CS with selector 0x08
    ; selector 0x08 = entry 1 (code segment), index 1 * 8
    jmp 0x08:.flush

.flush: ; Marks the jump target after the segment reload
    ret ; returns to the caller in C after GDT is loaded and segent registers are updated.