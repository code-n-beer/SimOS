/* blatantly stolen from http://ringzeroandlower.com/2017/08/08/x86-64-kernel-boot.html with minor tweaks */
ENTRY(start)

SECTIONS {
    . = 1M;
    _kernelPhysicalStart = .;

    .boottext : {
        build/boot/*(.multiboot .text)
    }

    .bootrodata : {
        build/boot/*(.rodata)
    }

    .bootdata : {
        build/boot/*(.data)
    }

    .bootbss : {
        build/boot/*(.bss COMMON) /* the vscode linker script plugin doesn't like
                                     COMMON so it mistakenly parses this input section as a comment */
    }

    . = ALIGN(1M);
    _bootEnd = .;

    . += 0xFFFFFFFF80000000;
    _kernelVirtualStart = .;

    .text : AT(_bootEnd) {
        *(.text)
    }

    .rodata ALIGN(4K) : AT(ALIGN(LOADADDR(.text) + SIZEOF(.text), 4K)) {
        *(.rodata*)
    }

    .data ALIGN(4K) : AT(ALIGN(LOADADDR(.rodata) + SIZEOF(.rodata), 4K)) {
        *(.data)
    }

    /* TODO: deal with this */
    .eh_frame ALIGN(4K) : AT(ALIGN(LOADADDR(.data) + SIZEOF(.data), 4K)) {
        *(.eh_frame)
    }

    .bss ALIGN(4K) : AT(ALIGN(LOADADDR(.eh_frame) + SIZEOF(.eh_frame), 4K)) {
        *(.bss COMMON)
    }

    . = ALIGN(4K);
    _kernelVirtualEnd = .;
    _kernelPhysicalEnd = ALIGN(LOADADDR(.bss) + SIZEOF(.bss), 4K);
}