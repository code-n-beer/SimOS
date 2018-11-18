#include <kernel.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <multiboot.h>
#include <printf.h>
#include <console.h>
#include <elf.h>

#define PML4_IDX_FROM_ADDR(addr)  (((addr) >> 39) & 0x1FF)
#define PDPT_IDX_FROM_ADDR(addr)  (((addr) >> 30) & 0x1FF)
#define PD_IDX_FROM_ADDR(addr)    (((addr) >> 21) & 0x1FF)

#define MMU_FLAG_PRESENT (1 << 0)
#define MMU_FLAG_READWRITE (1 << 1)

#define ASSERT(cond) \
    if (!(cond)) {\
        auto __vga_base = reinterpret_cast<uint16_t*>(0xb8000); \
        auto __i = 0; \
        for (auto __c : __FUNCTION__) { \
            __vga_base[__i++] = __c | 0x4f00; \
        } \
        __asm__("hlt"); \
    }

using PhysicalAddress = uintptr_t;

PhysicalAddress virtualToPhysical(void* ptr)
{
    auto addr = reinterpret_cast<uintptr_t>(ptr);
    auto pml4idx = PML4_IDX_FROM_ADDR(addr);
    auto pdptidx = PDPT_IDX_FROM_ADDR(addr);
    auto pdidx = PD_IDX_FROM_ADDR(addr);

    auto pml4 = reinterpret_cast<const uintptr_t*>(0xffff'ffff'ffff'f000ull + 8 * pml4idx);
    //printf("pml4idx: %lld (%llx)\n", pml4idx, *pml4);
    if (!(*pml4 & MMU_FLAG_PRESENT)) {
        return 0;
    }

    auto pdpt = reinterpret_cast<const uintptr_t*>(0xffff'ffff'ffe0'0000ull + 0x1000ull * pml4idx);
    //printf("pdptidx: %lld (%llx)\n", pdptidx, *pdpt);
    if (!(*pdpt & MMU_FLAG_PRESENT)) {
        return 0;
    }

    auto pd = reinterpret_cast<const uintptr_t*>(0xffff'ffff'c000'0000ull + 0x20'0000ull * pml4idx + 0x1000ull * pdptidx);
    //printf("pdidx: %lld (%llx)\n", pdidx, *pd);
    if (!(*pd & MMU_FLAG_PRESENT)) {
        return 0;
    }

    PhysicalAddress result = (*pd & (0x3fff'ffffull << 21)) | (addr & 0xf'ffffull);
    return result;
}

void dumpTag(const MmapTag& mmapTag)
{
    printf("Memory map:\n");
    ASSERT(mmapTag.entrySize == sizeof(MmapEntry));

    auto numEntries = (mmapTag.size - sizeof(MmapTag)) / mmapTag.entrySize;
    for (uint32_t i = 0; i < numEntries; i++) {
        const auto& entry = mmapTag.entries[i];
        printf("    addr: %016llx, len: %016llx, type: %d\n", entry.addr, entry.len, entry.type);
    }
}

void dumpTag(const ElfSectionsTag& tag)
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
        printf("%s (addr: %016llx, size: %llx)\n", strings + section.sh_name, section.sh_addr, section.sh_size);
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

void dumpMultibootInfo(const MultibootBasicInfo* info)
{
    for (const auto& tag : info) {
        switch (tag.type) {
        case TagType::Mmap:
            dumpTag(static_cast<const MmapTag&>(tag));
            break;
        case TagType::ElfSections:
            //dumpTag(static_cast<const ElfSectionsTag&>(tag));
            break;
        default:
            break;
        }
    }
}

static const size_t PAGE_SIZE = 0x20'0000;

enum MmuFlags : uint64_t
{
    Mmu_Present = (1 << 0),
    Mmu_ReadWrite = (1 << 1),
    Mmu_PageSize = (1 << 7)
};

void mapPhysical(PhysicalAddress physAddr, void* virtAddr, size_t size, uint64_t flags = 0)
{
    // Align to page size
    size = ((size - 1) & ~(PAGE_SIZE - 1)) + PAGE_SIZE;

    auto addr = reinterpret_cast<uintptr_t>(virtAddr);

    while (size) {
        auto pml4idx = PML4_IDX_FROM_ADDR(addr);
        auto pdptidx = PDPT_IDX_FROM_ADDR(addr);
        auto pdidx = PD_IDX_FROM_ADDR(addr);

        auto pml4 = reinterpret_cast<uint64_t*>(0xffff'ffff'ffff'f000ull + 8 * pml4idx);
        if (!(*pml4 & Mmu_Present)) {
            printf("pml4 index %lld not present!\n", pml4idx);
            return;
        }

        auto pdpt = reinterpret_cast<uint64_t*>(0xffff'ffff'ffe0'0000ull + 0x1000ull * pml4idx);
        if (!(*pdpt & Mmu_Present)) {
            printf("pdpt index %lld not present!\n", pdptidx);
            return;
        }

        auto pd = reinterpret_cast<uint64_t*>(0xffff'ffff'c000'0000ull + 0x20'0000ull * pml4idx + 0x1000ull * pdptidx);

        auto pde = (physAddr & (0x3fff'ffffull << 21)) | Mmu_Present | Mmu_PageSize | flags;
        printf("pde: %016llx\n", pde);

        addr += PAGE_SIZE;
        physAddr += PAGE_SIZE;
        size -= PAGE_SIZE;
    }
}

extern "C" void kmain(const MultibootBasicInfo* basicInfo)
{
    console::init();
    // 0000'7ffd'8c79'd000
    uint64_t* virtualPML4 = (uint64_t*)(0xFFFF'FFFF'FFFF'F000ULL);

    //void* p = (void*)(0xffff'ffff'0000'0000ull);
    void* p = (void*)(0xffff'ffff'0000'0000ull);
    auto x = (uint64_t)p;
    //mapPhysical(0x20'0000ull, p, 1 * PAGE_SIZE, Mmu_ReadWrite);
    printf("virt: %016llx, phys: %016llx\n", (uint64_t)p, virtualToPhysical(p));
    printf("virt: ffffffff8040a000, phys: %016llx\n", virtualToPhysical((void*)0xffff'ffff'8040'a000ull));
    dumpMultibootInfo(basicInfo);
    
    __asm__("hlt");
}