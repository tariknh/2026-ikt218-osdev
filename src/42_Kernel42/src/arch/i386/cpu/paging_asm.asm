global load_cr3
load_cr3:
    mov eax, [esp + 4] ; ESP + 0 is return address, so load 1st param at ESP+4
    mov cr3, eax ; move it into CR3
    ret

PG_BIT equ 0x80000000 ; paging enable bit

global enable_paging
enable_paging:
    mov eax, cr0 ;read current CR0 value
    ; merges PG=1 with existing bits
    or eax, PG_BIT
    mov cr0, eax ; write new CR0 value, enabling paging immediatly
    ret

global invalidate_page
invalidate_page:
    mov eax, [esp + 4] ; load first param
    invlpg [eax] ; invalidate page at given address
    ret

TSS_SEL equ 0x28

global tss_flush
tss_flush:
    mov ax, TSS_SEL
    ltr ax ; load task register 0x28, which is a Reload of the tss
    ret