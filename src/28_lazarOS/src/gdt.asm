; gdt.asm
;
; TODO: Export gdt_load and gdt_reload_segments with 'global'
;
; TODO: Implement gdt_load(gdt_descriptor_t *ptr)
;   Hint: argument is at [esp+4] (cdecl), use lgdt [eax]
;
; TODO: Implement gdt_reload_segments()
;   Hint: CS must be reloaded with a far jump to selector 0x08
;   Hint: Then set DS/ES/FS/GS/SS to 0x10

section .text
bits 32

