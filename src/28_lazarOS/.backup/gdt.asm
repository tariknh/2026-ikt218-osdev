; gdt.asm – low-level GDT loading routines
;
; Two functions are exported to C:
;   gdt_load(gdt_descriptor_t*)  – execute the lgdt instruction
;   gdt_reload_segments()        – far-jump to reload CS, then reload DS/ES/FS/GS/SS

global gdt_load
global gdt_reload_segments

section .text
bits 32

; void gdt_load(gdt_descriptor_t *descriptor)
;   The pointer to the 6-byte GDT descriptor is on the stack (cdecl calling convention).
gdt_load:
    mov  eax, [esp + 4]   ; load pointer argument from the stack
    lgdt [eax]            ; load the GDT descriptor into the GDTR register
    ret

; void gdt_reload_segments(void)
;   After lgdt the CPU still uses the old cached segment registers.
;   We must reload them to pick up the new GDT entries.
;
;   CS cannot be changed with a normal mov – a far jump is required.
;   The far jump below encodes:  jmp  0x08 : .reload_cs
;     0x08 = GDT_SELECTOR_CODE (index 1, TI=0, RPL=0)
gdt_reload_segments:
    jmp  0x08:.reload_cs    ; far jump flushes the instruction pipeline
                            ; and reloads CS with the code segment selector
.reload_cs:
    ; Reload all data-segment registers with the data segment selector
    ;   0x10 = GDT_SELECTOR_DATA (index 2, TI=0, RPL=0)
    mov  ax, 0x10
    mov  ds, ax
    mov  es, ax
    mov  fs, ax
    mov  gs, ax
    mov  ss, ax
    ret
