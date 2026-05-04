; gdt.asm - load GDT and reload segment registers
;
; The C function gdt_install() builds the GDT in memory, then calls this
; routine with a pointer to the GDT descriptor (limit + base).
;
; lgdt loads the descriptor into the CPU. After that we still have to
; reload every segment register so the CPU starts using our entries:
;   - data segments (DS, ES, FS, GS, SS) get loaded with selector 0x10
;     (index 2 in the GDT, since each entry is 8 bytes -> 2 * 8 = 0x10)
;   - the code segment (CS) cannot be assigned with mov; it has to be
;     reloaded by performing a far jump, which is why we jmp to the
;     selector 0x08 (index 1, 1 * 8 = 0x08).

global gdt_flush

section .text
bits 32

gdt_flush:
    mov     eax, [esp + 4]   ; argument: address of struct gdt_ptr
    lgdt    [eax]            ; load it into the GDTR

    ; reload data segment registers with our data selector (0x10)
    mov     ax, 0x10
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    mov     ss, ax

    ; reload code segment with our code selector (0x08) via a far jump
    jmp     0x08:.flush
.flush:
    ret
