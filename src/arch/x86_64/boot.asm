global start
global header_start
extern long_mode_start

section .multiboot
bits 32
header_start:
    dd 0xe85250d6                ; magic number (multiboot 2)
    dd 0                         ; architecture 0 (protected mode i386)
    dd header_end - header_start ; header length
    ; checksum
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))

    ; insert optional multiboot tags here

    ; required end tag
    dw 0    ; type
    dw 0    ; flags
    dd 8    ; size
header_end:

; page table entry macros
%define PRESENT     (1 << 0)
%define WRITABLE    (1 << 1)
%define HUGE        (1 << 7)
%define PAGE_SIZE   0x200000 ; 2MiB

%define PML4_IDX_FROM_ADDR(addr)  (((addr) >> 39) & 0x1FF)
%define PDPT_IDX_FROM_ADDR(addr)  (((addr) >> 30) & 0x1FF)
%define PD_IDX_FROM_ADDR(addr)    (((addr) >> 21) & 0x1FF)

%define KERNEL_VIRTUAL_START    0xFFFFFFFF80400000
%define KERNEL_PHYSICAL_START   0x0000000000400000

section .text
bits 32
start:
    mov esp, stack_top
    mov edi, ebx ; move multiboot header to edi

    call check_multiboot
    call check_cpuid
    call check_long_mode

    call set_up_page_tables
    call enable_paging

    lgdt [gdt64.pointer]
    jmp gdt64.code:long_mode_start

    hlt

error:
    mov dword [0xb8000], 0x4f524f45
    mov dword [0xb8004], 0x4f3a4f52
    mov dword [0xb8008], 0x4f204f20
    mov byte  [0xb800a], al
    hlt

; identity map the first gigabyte, eg. 0x00000000_00000000-0x00000000_40000000,
; then map 0xFFFFFFFF_80000000-0xFFFFFFFF_C0000000 to the first 1GiB of physical memory
set_up_page_tables:
    ; map appropriate PML4 entry for low PDPT
    mov eax, g_lowPDPT
    or eax, PRESENT | WRITABLE
    mov [g_PML4 + PML4_IDX_FROM_ADDR(KERNEL_PHYSICAL_START) * 8], eax

    ; map appropriate PML4 entry for high PDPT
    mov eax, g_highPDPT
    or eax, PRESENT | WRITABLE
    mov [g_PML4 + PML4_IDX_FROM_ADDR(KERNEL_VIRTUAL_START) * 8], eax

    ; map low PDPT entry to low PD
    mov eax, g_lowPD
    or eax, PRESENT | WRITABLE
    mov [g_lowPDPT + PDPT_IDX_FROM_ADDR(KERNEL_PHYSICAL_START) * 8], eax

    ; map high PDPT entry to high PD
    mov eax, g_highPD
    or eax, PRESENT | WRITABLE
    mov [g_highPDPT + PDPT_IDX_FROM_ADDR(KERNEL_VIRTUAL_START) * 8], eax


    ; map each low PD entry to a huge 2MiB page
    mov ecx, 0         ; counter variable

.map_lowPD:
    ; map ecx-th PD entry to a huge page that starts at address 2MiB*ecx
    mov eax, PAGE_SIZE
    mul ecx            ; start address of ecx-th page
    or eax, HUGE | PRESENT | WRITABLE
    mov [g_lowPD + ecx * 8], eax ; map ecx-th entry

    inc ecx            ; increase counter
    cmp ecx, 512       ; if counter == 512, the whole P2 table is mapped
    jne .map_lowPD  ; else map the next entry


    ; map each high PD entry to a huge 2MiB page
    mov ecx, 0         ; counter variable

.map_highPD:
    ; map ecx-th PD entry to a huge page that starts at address 2MiB*ecx
    mov eax, PAGE_SIZE
    mul ecx            ; start address of ecx-th page
    or eax, HUGE | PRESENT | WRITABLE
    mov [g_highPD + ecx * 8], eax ; map ecx-th entry

    inc ecx            ; increase counter
    cmp ecx, 512       ; if counter == 512, the whole P2 table is mapped
    jne .map_highPD  ; else map the next entry

    ret

check_multiboot:
    cmp eax, 0x36d76289
    jne .no_multiboot
    ret
.no_multiboot:
    mov al, "0"
    jmp error

check_cpuid:
    ; Check if CPUID is supported by attempting to flip the ID bit (bit 21)
    ; in the FLAGS register. If we can flip it, CPUID is available.

    ; Copy FLAGS in to EAX via stack
    pushfd
    pop eax

    ; Copy to ECX as well for comparing later on
    mov ecx, eax

    ; Flip the ID bit
    xor eax, 1 << 21

    ; Copy EAX to FLAGS via the stack
    push eax
    popfd

    ; Copy FLAGS back to EAX (with the flipped bit if CPUID is supported)
    pushfd
    pop eax

    ; Restore FLAGS from the old version stored in ECX (i.e. flipping the
    ; ID bit back if it was ever flipped).
    push ecx
    popfd

    ; Compare EAX and ECX. If they are equal then that means the bit
    ; wasn't flipped, and CPUID isn't supported.
    cmp eax, ecx
    je .no_cpuid
    ret
.no_cpuid:
    mov al, "1"
    jmp error

check_long_mode:
    ; test if extended processor info in available
    mov eax, 0x80000000    ; implicit argument for cpuid
    cpuid                  ; get lowest supported argument
    cmp eax, 0x80000001    ; it needs to be at least 0x80000001
    jb .no_long_mode       ; if it's less, the CPU is too old for long mode

    ; use extended info to test if long mode is available
    mov eax, 0x80000001    ; argument for extended processor info
    cpuid                  ; returns various feature bits in ecx and edx
    test edx, 1 << 29      ; test if the LM-bit is set in the D-register
    jz .no_long_mode       ; If it's not set, there is no long mode
    ret
.no_long_mode:
    mov al, "2"
    jmp error

enable_paging:
    ; load P4 to cr3 register (cpu uses this to access the P4 table)
    mov eax, g_PML4
    mov cr3, eax

    ; enable PAE-flag in cr4 (Physical Address Extension)
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; set the long mode bit in the EFER MSR (model specific register)
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; enable paging in the cr0 register
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    ret

section .rodata
gdt64:
    ;dq means define quad, outputs 64-bit constant
    dq 0 ; zero entry
.code: equ $ - gdt64
    dq (1<<43) | (1<<44) | (1<<47) | (1<<53) ; code segment
.pointer:
    dw $ - gdt64 - 1
    dq gdt64

section .bss

global g_PML4
global g_lowPDPT
global g_lowPD
global g_highPDPT
global g_highPD

align 4096
g_PML4:
    resb 4096
g_highPDPT:
    resb 4096
g_highPD:
    resb 4096
g_lowPDPT:
    resb 4096
g_lowPD:
    resb 4096

stack_bottom:
    resb 4096 * 4
stack_top:
