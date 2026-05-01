; gdt_flush.asm
; Laster GDTR med lgdt og reloader segmentregister (i386, 32-bit)

BITS 32
global gdt_flush

gdt_flush:
    mov eax, [esp+4]      ; argument: adresse til gdt_ptr/gdtr struct
    lgdt [eax]

    ; DATA selector = 0x10 (GDT entry 2)
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; CODE selector = 0x08 (GDT entry 1)
    jmp 0x08:.reload_cs
.reload_cs:
    ret
