/* blatantly stolen from http://ringzeroandlower.com/2017/08/08/x86-64-kernel-boot.html with minor tweaks */
ENTRY(start)

SECTIONS {
    . = SIZEOF_HEADERS;

    .boottext : {
        */src_boot*.S.o(.text)
    }

    .bootrodata : {
        */src_boot*.S.o(.rodata)
    }

    .bootdata : {
        */src_boot*.S.o(.data)
        */src_boot*.S.o(.multiboot)
    }

    .bootbss : {
        */src_boot*.S.o(.bss COMMON)
    }

    . = 0xB8000;

    .bootsplash : {
        KEEP(*(.bootsplash))
    }

    . = ALIGN(2M);
    _kernelPhysicalStart = .;
    _kernelStackBottomPA = .;
    _kernelStackTopPA = _kernelStackBottomPA - 16K;

    . += 0xFFFFFFFF80000000;
    _kernelStackBottomVA = .;
    _kernelStackTopVA = _kernelStackBottomVA - 16K;
    _kernelVirtualStart = .;

    .text : AT(_kernelPhysicalStart) {
        *(.text*)
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