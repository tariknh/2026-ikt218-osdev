global gdt_flush    ; make the symbol visible to C code

section .text
bits 32


gdt_flush:
    mov  eax, [esp+4]    ; load the gdt_descriptor address from the stack
    lgdt [eax]           ; tell the CPU about the new GDT

    jmp  0x08:.reload_cs

.reload_cs:
    mov  ax, 0x10
    mov  ds, ax
    mov  es, ax
    mov  fs, ax
    mov  gs, ax
    mov  ss, ax

    ret
