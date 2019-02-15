/* page table entry macros */
#define PRESENT     (1 << 0)
#define WRITABLE    (1 << 1)
#define HUGE        (1 << 7)
#define PAGE_SIZE   0x200000 /* 2MiB */

#define PML4_IDX_FROM_ADDR(addr)  (((addr) >> 39) & 0x1FF)
#define PDPT_IDX_FROM_ADDR(addr)  (((addr) >> 30) & 0x1FF)
#define PD_IDX_FROM_ADDR(addr)    (((addr) >> 21) & 0x1FF)

#define KERNEL_VIRTUAL_START    0xFFFFFFFF80200000
#define KERNEL_PHYSICAL_START   0x0000000000100000

#define VGA_BASE 0xB8000

/*
    LongMode = stl::bit(53),
    Present = stl::bit(47),
    CodeOrData = stl::bit(44),
    CodeSegment = stl::bit(43),
    DataWritable = stl::bit(41),
    CodeReadable = stl::bit(41),
*/
#define GDT_LONG_MODE (1 << 53)
#define GDT_PRESENT (1 << 47)
#define GDT_CODE_OR_DATA (1 << 44)
#define GDT_CODE_SEGMENT (1 << 43)

.section .bss
.align 4096

.LPML4:
    .space 4096
.LPDPT:
    .space 4096
.Lstack:
    .space 4096
.LstackTop:

.data
.align 16
.LGDT:
    .quad 0 /* null descriptor */
    .quad GDT_LONG_MODE | GDT_PRESENT | GDT_CODE_OR_DATA | GDT_CODE_SEGMENT /* code segment */
.LGDTR:
    .word . - .LGDT - 1
    .long .LGDT

.code32
.section .text
.global start

start:
    movl $.Lstack, %esp
    movl %ebx, %edi

    /* identity map the first 1GiB of physical memory, and then map it to
       0xFFFFFFFF_80000000 as well */

    /* build PML4 entries */
    movl $.LPDPT, %eax
    orl $(PRESENT | WRITABLE), %eax

    movl $PML4_IDX_FROM_ADDR(KERNEL_PHYSICAL_START), %ebx
    movl $PML4_IDX_FROM_ADDR(KERNEL_VIRTUAL_START), %ecx
    movl %eax, .LPML4(, %ebx, 8)
    movl %eax, .LPML4(, %ecx, 8)

    /* doing this instead of the above will cause the compiler to use movabs... */
    /*movl %eax, PML4 + (PML4_IDX_FROM_ADDR(KERNEL_PHYSICAL_START) * 8)
    movl %eax, PML4 + (PML4_IDX_FROM_ADDR(KERNEL_VIRTUAL_START) * 8)*/

    /* build PDPT entries */
    movl $0, %eax
    orl $(PRESENT | WRITABLE | HUGE), %eax

    movl $PDPT_IDX_FROM_ADDR(KERNEL_PHYSICAL_START), %ebx
    movl $PDPT_IDX_FROM_ADDR(KERNEL_VIRTUAL_START), %ecx
    movl %eax, .LPDPT(, %ebx, 8)
    movl %eax, .LPDPT(, %ecx, 8)

    /*movl %eax, PDPT + (PDPT_IDX_FROM_ADDR(KERNEL_PHYSICAL_START) * 8)
    movl %eax, PDPT + (PDPT_IDX_FROM_ADDR(KERNEL_VIRTUAL_START) * 8)*/

    /* recursively map second-last entry of PML4 */
    movl $.LPML4, %eax
    orl $(PRESENT | WRITABLE), %eax
    movl $510, %ebx
    movl %eax, .LPML4(, %ebx, 8)

    /* load the PML4 address in CR3 */
    movl $.LPML4, %eax
    movl %eax, %cr3

    nop
    nop

    /* set the PAE flag in CR4 */
    movl %cr4, %eax
    orl $(1 << 5), %eax
    movl %eax, %cr4

    /* set IA32_EFER.LME */
    movl $0xC0000080, %ecx
    rdmsr
    orl $(1 << 8), %eax
    wrmsr

    /* set the paging enable flag in CR0 */
    movl %cr0, %eax
    orl $(1 << 31), %eax
    movl %eax, %cr0

    /* load the GDT */
    lgdt .LGDTR

    /* reload CS */
    ljmp $0x08, $long_mode_start

    hlt

error:
    movl $0x4f524f45, (0xb8000)
    movl $0x4f3a4f52, (0xb8004)
    movl $0x4f204f20, (0xb8008)
    movb %al, (0xb800a)

    hlt