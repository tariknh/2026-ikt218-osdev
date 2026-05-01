; gdt.asm
; loads the GDT into the CPU and reloads all segment registers
; we need this in assembly because lgdt and far jumps cant be done in C

global gdt_flush

; called from gdt.c with the address of our gdt_descriptor
; loads the GDT and then reloads all segment registers
gdt_flush:
    ; get the gdt_descriptor address from the stack (first argument from C)
    mov eax, [esp + 4]

    ; load the GDT into the CPU
    lgdt [eax]

    ; reload the code segment using a far jump
    ; 0x08 is our kernel code segment (entry 1 x 8 bytes)
    jmp 0x08:.reload_cs

.reload_cs:
    ; reload all data segment registers
    ; 0x10 is our kernel data segment (entry 2 x 8 bytes)
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret