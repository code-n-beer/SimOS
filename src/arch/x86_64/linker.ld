/* blatantly stolen from http://ringzeroandlower.com/2017/08/08/x86-64-kernel-boot.html with minor tweaks */
ENTRY(start)

SECTIONS
{
    . = 1M;
    _kernelPhysicalStart = .;

    .boottext :
    {
        build/arch/x86_64/boot.o(.multiboot)
        build/arch/x86_64/boot.o(.text)
        build/arch/x86_64/long_mode_init.o(.text)
    }

    .bootrodata :
    {
        build/arch/x86_64/boot.o(.rodata)
        build/arch/x86_64/long_mode_init.o(.rodata)
    }

    .bootdata :
    {
        build/arch/x86_64/boot.o(.data)
        build/arch/x86_64/long_mode_init.o(.data)
    }

    .bootbss :
    {
        build/arch/x86_64/boot.o(.bss)
        build/arch/x86_64/boot.o(COMMON)
        build/arch/x86_64/long_mode_init.o(.bss)
        build/arch/x86_64/long_mode_init.o(COMMON)
    }

    . = ALIGN(0x1000);
    _bootEnd = .;

    . += 0xFFFFFFFF80000000;
    _kernelVirtualStart = .;

    .text : AT(_bootEnd)
    {
        *(.multiboot)
        *(.text)
    }

    . = ALIGN(0x1000);
    .rodata : AT ( (LOADADDR (.text) + SIZEOF (.text) + 0xFFF) & 0xFFFFFFFFFFFFF000 )
    {
        *(.rodata)
    }

    . = ALIGN(0x1000);
    .data : AT ( (LOADADDR (.rodata) + SIZEOF (.rodata) + 0xFFF) & 0xFFFFFFFFFFFFF000 )
    {
        *(.data)
    }

    . = ALIGN(0x1000);
    .bss : AT ( (LOADADDR (.data) + SIZEOF (.data) + 0xFFF) & 0xFFFFFFFFFFFFF000 )
    {
        *(COMMON)
        *(.bss)
    }

    _kernelVirtualEnd = .;
    _kernelPhysicalEnd = (LOADADDR (.bss) + SIZEOF (.bss) + 0xFFF) & 0xFFFFFFFFFFFFF000;
}