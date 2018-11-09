/* blatantly stolen from http://ringzeroandlower.com/2017/08/08/x86-64-kernel-boot.html with minor tweaks */
ENTRY(start)

SECTIONS
{
    . = 4M;
    _kernel_physical_start = .;

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
    _boot_end = .;

    . += 0xFFFFFFFF80000000;
    _kernel_virtual_start = .;
    .text : AT(_boot_end)
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

    _kernel_virtual_end = .;

    _kernel_physical_end = (LOADADDR (.bss) + SIZEOF (.bss) + 0xFFF) & 0xFFFFFFFFFFFFF000;
}