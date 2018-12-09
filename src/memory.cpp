#include <simo/kernel.h>
#include <simo/multiboot.h>
#include <simo/memory.h>
#include <simo/pagemap.h>
#include <simo/utils.h>
#include <printf.h>

namespace memory
{

extern "C" uint8_t _kernelVirtualStart;
extern "C" uint8_t _kernelVirtualEnd;
extern "C" uint8_t _kernelPhysicalStart;
extern "C" uint8_t _kernelPhysicalEnd;
extern "C" uint8_t _bootEnd;
extern "C" uint8_t _stack_bottom;

const size_t PAGE_SIZE = 0x1000;

constexpr PhysicalAddress alignToPage(PhysicalAddress addr)
{
    return ((addr - 1) + PAGE_SIZE) & ~(PAGE_SIZE - 1);
}

class PhysicalFrameMap
{
public:
    PhysicalFrameMap(PhysicalAddress memoryBase, size_t memorySize) :
        m_memoryBase(memoryBase), 
        m_bitmapSize((memorySize / PAGE_SIZE) / (sizeof(uint64_t) * 8))
    {
        memset(m_bitmap, 0, m_bitmapSize * sizeof(uint64_t));
    }

    PhysicalAddress getNextFreeFrame()
    {
        auto addr = m_memoryBase;

        // TODO: make this not stupid
        while (true) {
            if (!isFrameUsed(addr)) {
                return addr;
            }

            addr += PAGE_SIZE;
        }

        return 0ull;
    }

    void markFrame(PhysicalAddress address, bool used)
    {
        const auto frameIdx = ((address - m_memoryBase) / PAGE_SIZE);
        const auto entryIdx = frameIdx / (sizeof(uint64_t) * 8);
        const auto shift = frameIdx % (sizeof(uint64_t) * 8);
        const auto mask = 1UL << shift;

        // TODO: assert(entryIdx < m_bitmapSize);
        // TODO: what if there are multiple mappings to the same frame and only one of them is unmapped and freed?

        if (used) {
            m_bitmap[entryIdx] |= mask;
        } else {
            m_bitmap[entryIdx] &= ~mask;
        }
    }

    bool isFrameUsed(PhysicalAddress address) const
    {
        const auto frameIdx = ((address - m_memoryBase) / PAGE_SIZE);
        const auto entryIdx = frameIdx / (sizeof(uint64_t) * 8);
        const auto shift = frameIdx % (sizeof(uint64_t) * 8);
        const auto mask = 1UL << shift;

        // TODO: assert(entryIdx < m_bitmapSize);

        return (m_bitmap[entryIdx] & mask) != 0;
    }

    size_t getBitmapSize() const
    {
        return m_bitmapSize;
    }

    size_t getByteSize() const
    {
        return sizeof(*this) + m_bitmapSize * sizeof(uint64_t);
    }

private:
    PhysicalAddress m_memoryBase;
    size_t m_bitmapSize;
    uint64_t m_bitmap[0];
};

MmapEntry findBiggestMemoryArea(const MmapTag* mmap)
{
    MmapEntry biggest{};

    auto numEntries = (mmap->size - sizeof(MmapTag)) / mmap->entrySize;
    for (auto i = 0ul; i < numEntries; i++) {
        const auto& entry = mmap->entries[i];
        if (entry.type == MemoryType::Available && entry.len > biggest.len) {
            biggest = entry;
        }
    }

    return biggest;
}

// the name is pretty misleading, there's no guarantee that there's actual physical memory after
// the multiboot structure, but whatever
PhysicalAddress getFirstSafePhysicalAddress(const MultibootBasicInfo* multibootInfo)
{
    auto physAddr = reinterpret_cast<PhysicalAddress>(multibootInfo);
    physAddr += multibootInfo->totalSize;

    return alignToPage(physAddr);
}

PhysicalFrameMap* initPhysicalFrameMap(const MmapTag* mmap, void* addr)
{
    const auto mem = findBiggestMemoryArea(mmap);
    printf("Physical memory at %016lx (size 0x%lx)\n", mem.addr, mem.len);
    return new (addr) PhysicalFrameMap(mem.addr, mem.len);
}

template<typename T1, typename T2>
struct Pair
{
    T1 first;
    T2 second;
};

Pair<const ElfSectionsTag*, const MmapTag*> getMultibootTags(const MultibootBasicInfo* multibootInfo)
{
    const ElfSectionsTag* elfSections = nullptr;
    const MmapTag* memoryMap = nullptr;

    for (const auto& tag : multibootInfo) {
        if (tag.type == TagType::ElfSections) {
            elfSections = static_cast<const ElfSectionsTag*>(&tag);
        } else if (tag.type == TagType::Mmap) {
            memoryMap = static_cast<const MmapTag*>(&tag);
        }
    }

    return { elfSections, memoryMap };
}

PhysicalFrameMap* g_physFrameMap = nullptr;

void setupPageTables(const MultibootBasicInfo* multibootInfo)
{
    auto [elfSections, memoryMap] = getMultibootTags(multibootInfo);
    // TODO: assert(elfSections && memoryMap);

    auto physFrameMapPA = getFirstSafePhysicalAddress(multibootInfo);
    // virtual address is physical + 0xffffffff80000000 at this point
    auto physFrameMapVA = reinterpret_cast<void*>(physFrameMapPA + 0xffff'ffff'8000'0000ull);

    g_physFrameMap = initPhysicalFrameMap(memoryMap, reinterpret_cast<void*>(physFrameMapVA));
    const auto physFrameMapEndPA = alignToPage(physFrameMapPA + g_physFrameMap->getByteSize());

    auto startPtr = reinterpret_cast<PhysicalAddress>(&_kernelPhysicalStart);
    auto endPtr = reinterpret_cast<PhysicalAddress>(&_kernelPhysicalEnd);

    // reserve the physical memory where the kernel was loaded
    printf("Reserving %016lx-%016lx...\n", startPtr, endPtr);
    for (auto ptr = startPtr; ptr < endPtr; ptr += 0x1000) {
        g_physFrameMap->markFrame(reinterpret_cast<PhysicalAddress>(ptr), true);
    }

    // reserve the physical memory used by the frame allocator
    printf("Reserving %016lx-%016lx...\n", physFrameMapPA, physFrameMapEndPA);
    for (auto ptr = physFrameMapPA; ptr < physFrameMapEndPA; ptr += 0x1000) {
        g_physFrameMap->markFrame(ptr, true);
    }

    // TODO: getNextFreePage + markPage -> PhysicalPageMap::allocateFrame
    auto pml4PA = g_physFrameMap->getNextFreeFrame();
    auto pml4 = new (reinterpret_cast<void*>(pml4PA)) PML4();
    g_physFrameMap->markFrame(pml4PA, true);
    printf("PML4 is at %016lx\n", pml4PA);

    auto pdptPA = g_physFrameMap->getNextFreeFrame();
    auto pdpt = new (reinterpret_cast<void*>(pdptPA)) PDPT();
    printf("PDPT is at %016lx\n", pdptPA);
    g_physFrameMap->markFrame(pdptPA, true);

    auto pdpt2PA = g_physFrameMap->getNextFreeFrame();
    auto pdpt2 = new (reinterpret_cast<void*>(pdpt2PA)) PDPT();
    printf("PDPT2 is at %016lx\n", pdpt2PA);
    g_physFrameMap->markFrame(pdpt2PA, true);

    auto pdPA = g_physFrameMap->getNextFreeFrame();
    auto pd = new (reinterpret_cast<void*>(pdPA)) PD();
    printf("PD is at %016lx\n", pdPA);
    g_physFrameMap->markFrame(pdPA, true);

    // TODO: finish this
    auto va = (void*)0xffff'ffff'8000'0000ull;
    auto va2 = (void*)0x0000'0000'000b'8000ull;
    pml4->entryFromAddress(va) = { pdptPA | PML4E::Present | PML4E::Write };
    pdpt->entryFromAddress(va) = { 0UL | PDPTE::PageSize | PDPTE::Present | PDPTE::Write };

    pml4->entryFromAddress(va2) = { pdpt2PA | PML4E::Present | PML4E::Write };
    pdpt2->entryFromAddress(va2) = { pdPA | PDPTE::Present | PDPTE::Write };
    pd->entryFromAddress(va2) = { 0UL | PDE::Present | PDE::PageSize | PDE::Write };

    asm volatile(
        "movq %0, %%rax\n"
        "movq %%rax, %%cr3\n"
        : : "r"(pml4PA) : "%rax"
    );

    printf("No longer running with identity mapping \\:D/\n");
}

void init(const MultibootBasicInfo* multibootInfo)
{
    setupPageTables(multibootInfo);
    printf("if you're reading this, memory mapping actually work\n");
}

}