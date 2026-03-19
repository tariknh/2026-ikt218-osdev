; gdt.asm
; Loads the GDT into the CPU and reloads all segment registers.
; This file is needed because the lgdt instruction and far jump
; cannot be done directly from C code.

global gdt_flush   ; Make gdt_flush visible to C code

; gdt_flush(uint32_t gdt_descriptor_address)
; 
; Called from gdt.c with the address of our gdt_descriptor struct.
; Loads the GDT into the CPU's GDTR register, then reloads
; all segment registers so the CPU uses the new GDT.
gdt_flush:
    ; Get the address of the GDT descriptor from the stack
    ; (first argument passed from C)
    mov eax, [esp + 4]

    ; Load the GDT into the CPU using the lgdt instruction
    lgdt [eax]

    ; Reload the code segment register (CS) using a far jump
    ; 0x08 is our kernel code segment selector (entry 1 x 8 bytes)
    jmp 0x08:.reload_cs

.reload_cs:
    ; Reload all data segment registers
    ; 0x10 is our kernel data segment selector (entry 2 x 8 bytes)
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret