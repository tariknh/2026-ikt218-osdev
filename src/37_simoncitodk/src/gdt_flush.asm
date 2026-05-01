global gdt_flush
extern gp

section .text

gdt_flush:
    lgdt [gp]

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    jmp 0x08:.reload_cs

.reload_cs:
    ret