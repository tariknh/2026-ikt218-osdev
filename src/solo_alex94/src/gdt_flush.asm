global gdt_flush

section .text
bits 32

gdt_flush:
    ; Function argument: pointer to the gdt_ptr structure
    mov eax, [esp + 4]

    ; Load the new GDT into the processor
    lgdt [eax]

    ; Reload CS with the code segment selector
    jmp 0x08:reload_segments

reload_segments:
    ; Reload data segment registers with the data selector
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ret
