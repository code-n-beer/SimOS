global long_mode_start

extern kmain
extern _init
extern _fini
extern _kernelStackBottomVA

section .text
bits 64
long_mode_start:
    ; load 0 into all data segment registers
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    lea rsp, [_kernelStackBottomVA]

    call _init
    call kmain
    call _fini

    hlt