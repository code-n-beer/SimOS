#include <simo/kernel.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <simo/multiboot.h>
#include <printf.h>
#include <simo/console.h>
#include <simo/elf.h>
#include <simo/paging.h>
#include <stl/lambda.h>
#include <simo/interrupt.h>
#include <simo/gdt.h>

void dumpTag(const multiboot::MmapTag& mmapTag)
{
    printf("Memory map:\n");

    auto numEntries = (mmapTag.size - sizeof(multiboot::MmapTag)) / mmapTag.entrySize;
    for (uint32_t i = 0; i < numEntries; i++) {
        const auto& entry = mmapTag.entries[i];
        printf("    addr: %016lx, len: %016lx, type: %d\n", entry.addr, entry.len, int(entry.type));
    }
}

void dumpTag(const multiboot::ElfSectionsTag& tag)
{
    /*
    Elf64_Word  sh_name;    // Section name (index into the section header string table).
    Elf64_Word  sh_type;    // Section type.
    Elf64_Xword sh_flags;   // Section flags.
    Elf64_Addr  sh_addr;    // Address in memory image.
    Elf64_Off   sh_offset;  // Offset in file. 
    Elf64_Xword sh_size;    // Size in bytes.  
    Elf64_Word  sh_link;    // Index of a related section.
    Elf64_Word  sh_info;    // Depends on section type. 
    Elf64_Xword sh_addralign;   // Alignment in bytes.  
    Elf64_Xword sh_entsize; // Size of each entry in section.
    */
    const Elf64_Shdr* sections = reinterpret_cast<const Elf64_Shdr*>(&tag.sections);
    const Elf64_Shdr* stringTable = &sections[tag.shndx];
    const char* strings = reinterpret_cast<const char*>(stringTable->sh_addr);

    printf("ELF sections\n");
    for (uint32_t i = 0; i < tag.num; i++) {
        const auto& section = sections[i];
        printf("%s (addr: %016lx, size: %lx)\n", strings + section.sh_name, section.sh_addr, section.sh_size);
        /*printf("  sh_name:\t%s\n", strings + section.sh_name);
        printf("  sh_type:\t%08x\n", section.sh_type);
        printf("  sh_flags:\t%08x\n", section.sh_flags);
        printf("  sh_addr:\t%016llx\n", section.sh_addr);
        printf("  sh_offset:\t%016llx\n", section.sh_offset);
        printf("  sh_size:\t%016llx\n", section.sh_size);
        printf("  sh_link:\t%08x\n", section.sh_link);
        printf("  sh_info:\t%08x\n", section.sh_info);
        printf("  sh_addralign:\t%016llx\n", section.sh_addralign);
        printf("  sh_entsize:\t%016llx\n\n", section.sh_entsize);*/
    }
}

void dumpMultibootInfo(const multiboot::Info* info)
{
    for (const auto& tag : info) {
        switch (tag.type) {
        case multiboot::TagType::Mmap:
            dumpTag(static_cast<const multiboot::MmapTag&>(tag));
            break;
        case multiboot::TagType::ElfSections:
            //dumpTag(static_cast<const ElfSectionsTag&>(tag));
            break;
        default:
            break;
        }
    }
}

extern "C" void kmain(const multiboot::Info* info)
{
    console::init();
    paging::init(info);
    gdt::init();
    interrupts::init();

    printf("forcing a page fault: *(int*)(0xdeadbeef) = 0xbadc0de; ...\n");
    *(int*)(0xdeadbeef) = 0xbadc0de;
    asm volatile("hlt");
}