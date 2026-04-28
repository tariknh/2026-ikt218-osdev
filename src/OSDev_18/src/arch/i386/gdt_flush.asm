%define GDT_CODE_SELECTOR 0x08
%define GDT_DATA_SELECTOR 0x10

global GdtFlush

section .text
bits 32

GdtFlush:
    mov eax, [esp + 4]
    lgdt [eax]

    ; Reload cs from the new table via a far jump.
    jmp GDT_CODE_SELECTOR:.reload_cs

.reload_cs:
    ; The remaining segment registers still cache the old descriptors.
    mov ax, GDT_DATA_SELECTOR
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret
