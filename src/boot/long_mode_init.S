.global long_mode_start

.extern kmain
.extern _init
.extern _fini
.extern _kernelStackBottomVA

.text
long_mode_start:
    # load 0 into all data segment registers
    movw $0, %ax
    mov %ax, %ss
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    lea _kernelStackBottomVA(,1), %rsp

    call _init
    call kmain
    call _fini

    hlt