#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <multiboot2.h>
#include <printf.h>
#include <console.h>

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
    printf("pml4idx: %lld (%llx)\n", pml4idx, *pml4);
    if (!(*pml4 & MMU_FLAG_PRESENT)) {
        return 0;
    }

    auto pdpt = reinterpret_cast<const uintptr_t*>(0xffff'ffff'ffe0'0000ull + 0x1000ull * pml4idx);
    printf("pdptidx: %lld (%llx)\n", pdptidx, *pdpt);
    if (!(*pdpt & MMU_FLAG_PRESENT)) {
        return 0;
    }

    auto pd = reinterpret_cast<const uintptr_t*>(0xffff'ffff'c000'0000ull + 0x20'0000ull * pml4idx + 0x1000ull * pdptidx);
    printf("pdidx: %lld (%llx)\n", pdidx, *pd);
    if (!(*pd & MMU_FLAG_PRESENT)) {
        return 0;
    }

    PhysicalAddress result = (*pd & (0x3fff'ffffull << 21)) | (addr & 0xf'ffffull);
    return result;
}

extern "C" void kmain(uint64_t* multibootHeader)
{
    console::init();

    //uint64_t* virtualPML4 = (uint64_t*)(0xFFFFFFFFFFE00000ULL);
    uint64_t* virtualPML4 = (uint64_t*)(0xFFFF'FFFF'FFFF'F000ULL);
    //ASSERT(false);

    printf("PML4 dump:\n\n");
    for (int i = 0; i < 512; i++) {
        printf("PML4[%d]: %016llx\n", i, virtualPML4[i]);
    }
    printf("kmain virtual:\t%016llx\n", (uint64_t)kmain);
    printf("kmain physical:\t%016llx\n", virtualToPhysical((void*)kmain));
    
    while (true) {
    }
}