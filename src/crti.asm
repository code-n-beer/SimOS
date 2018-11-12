
section .init
global _init
bits 64
_init:
    push rbp
    mov rbp, rsp

section .fini
global _fini
bits 64
_fini:
    push rbp
    mov rbp, rsp