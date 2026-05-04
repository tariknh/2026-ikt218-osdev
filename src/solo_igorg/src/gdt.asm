global gdt_flush

; Loads GDT  and reloads segment registers.
;
; C prototype:
;   void gdt_flush(uint32_t gdt_ptr_address);
;
; Stack layout when called from C:
;   [esp + 4] = address of struct gdt_ptr
;
; Segment selectors:
;   0x08 = index 1 in the GDT = kernel code segment
;   0x10 = index 2 in the GDT = kernel data segment

gdt_flush:
    mov eax, [esp + 4]
    lgdt [eax]

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Reload CS with a far jump.
    jmp 0x08:.reload_code_segment

.reload_code_segment:
    ret
