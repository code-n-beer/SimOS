#include <simo/kernel.h>
#include <simo/multiboot.h>
#include <simo/memory.h>
#include <simo/pagemap.h>
#include <simo/utils.h>
#include <printf.h>
#include <stl/tuple.h>

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
    return PhysicalAddress { ((static_cast<uint64_t>(addr) - 1) + PAGE_SIZE) & ~(PAGE_SIZE - 1) };
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

        return PhysicalAddress::Null;
    }

    void markFrame(PhysicalAddress address, bool used)
    {
        auto [entry, mask] = computeEntryIndexAndMask(address);

        // TODO: assert(entryIdx < m_bitmapSize);
        // TODO: what if there are multiple mappings to the same frame and only one of them is unmapped and freed?

        if (used) {
            m_bitmap[entry] |= mask;
        } else {
            m_bitmap[entry] &= ~mask;
        }
    }

    bool isFrameUsed(PhysicalAddress address) const
    {
        auto [entry, mask] = computeEntryIndexAndMask(address);

        // TODO: assert(entryIdx < m_bitmapSize);

        return (m_bitmap[entry] & mask) != 0;
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
    stl::Tuple<size_t, uint64_t> computeEntryIndexAndMask(PhysicalAddress address) const
    {
        const auto frameIdx = ((static_cast<uint64_t>(address) - static_cast<uint64_t>(m_memoryBase)) / PAGE_SIZE);
        const auto entryIdx = frameIdx / (sizeof(uint64_t) * 8);
        const auto shift = frameIdx % (sizeof(uint64_t) * 8);
        const auto mask = 1UL << shift;

        return {entryIdx, mask};
    }

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

PhysicalAddress identityMappedVirtualToPhysical(const void* addr)
{
    return PhysicalAddress{reinterpret_cast<uint64_t>(addr)};
}

template<typename T = void>
T* identityMappedPhysicalToVirtual(PhysicalAddress addr)
{
    return reinterpret_cast<T*>(static_cast<uint64_t>(addr));
}

// the name is pretty misleading, there's no guarantee that there's actual physical memory after
// the multiboot structure, but whatever
PhysicalAddress getFirstSafePhysicalAddress(const MultibootBasicInfo* multibootInfo)
{
    auto physAddr = identityMappedVirtualToPhysical(multibootInfo);
    physAddr += multibootInfo->totalSize;

    return alignToPage(physAddr);
}

PhysicalFrameMap* initPhysicalFrameMap(const MmapTag* mmap, void* addr)
{
    const auto mem = findBiggestMemoryArea(mmap);
    printf("Physical memory at %016lx (size 0x%lx)\n", mem.addr, mem.len);
    return new (addr) PhysicalFrameMap(PhysicalAddress{mem.addr}, mem.len);
}

stl::Tuple<const ElfSectionsTag*, const MmapTag*> getMultibootTags(const MultibootBasicInfo* multibootInfo)
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

    return {elfSections, memoryMap};
}

PhysicalFrameMap* g_physFrameMap = nullptr;

void setupPageTables(const MultibootBasicInfo* multibootInfo)
{
    auto [elfSections, memoryMap] = getMultibootTags(multibootInfo);
    // TODO: assert(elfSections && memoryMap);

    auto physFrameMapPA = getFirstSafePhysicalAddress(multibootInfo);
    // virtual address is physical + 0xffffffff80000000 at this point
    auto physFrameMapVA = identityMappedPhysicalToVirtual(physFrameMapPA + 0xffff'ffff'8000'0000ull);

    g_physFrameMap = initPhysicalFrameMap(memoryMap, physFrameMapVA);
    const auto physFrameMapEndPA = alignToPage(physFrameMapPA + g_physFrameMap->getByteSize());

    auto startPtr = identityMappedVirtualToPhysical(&_kernelPhysicalStart);
    auto endPtr = identityMappedVirtualToPhysical(&_kernelPhysicalEnd);

    // reserve the physical memory where the kernel was loaded
    printf("Reserving %016lx-%016lx...\n", uint64_t(startPtr), uint64_t(endPtr));
    for (auto ptr = startPtr; ptr < endPtr; ptr += 0x1000) {
        g_physFrameMap->markFrame(ptr, true);
    }

    // reserve the physical memory used by the frame allocator
    printf("Reserving %016lx-%016lx...\n", uint64_t(physFrameMapPA), uint64_t(physFrameMapEndPA));
    for (auto ptr = physFrameMapPA; ptr < physFrameMapEndPA; ptr += 0x1000) {
        g_physFrameMap->markFrame(ptr, true);
    }

    // TODO: getNextFreePage + markPage -> PhysicalPageMap::allocateFrame
    auto pml4PA = g_physFrameMap->getNextFreeFrame();
    auto pml4 = new (identityMappedPhysicalToVirtual(pml4PA)) PML4();
    g_physFrameMap->markFrame(pml4PA, true);
    printf("PML4 is at %016lx\n", uint64_t(pml4PA));

    auto pdptPA = g_physFrameMap->getNextFreeFrame();
    auto pdpt = new (identityMappedPhysicalToVirtual(pdptPA)) PDPT();
    printf("PDPT is at %016lx\n", uint64_t(pdptPA));
    g_physFrameMap->markFrame(pdptPA, true);

    auto pdpt2PA = g_physFrameMap->getNextFreeFrame();
    auto pdpt2 = new (identityMappedPhysicalToVirtual(pdpt2PA)) PDPT();
    printf("PDPT2 is at %016lx\n", uint64_t(pdpt2PA));
    g_physFrameMap->markFrame(pdpt2PA, true);

    auto pdPA = g_physFrameMap->getNextFreeFrame();
    auto pd = new (identityMappedPhysicalToVirtual(pdPA)) PD();
    printf("PD is at %016lx\n", uint64_t(pdPA));
    g_physFrameMap->markFrame(pdPA, true);

    // TODO: finish this
    auto va = (void*)0xffff'ffff'8000'0000ull;
    auto va2 = (void*)0x0000'0000'000b'8000ull;
    pml4->entryFromAddress(va).set(pdptPA, PMEFlags::Present, PMEFlags::Write);
    pdpt->entryFromAddress(va).set(PhysicalAddress::Null, PMEFlags::PageSize, PMEFlags::Present, PMEFlags::Write);

    pml4->entryFromAddress(va2).set(pdpt2PA, PMEFlags::Present, PMEFlags::Write);
    pdpt2->entryFromAddress(va2).set(pdPA, PMEFlags::Present, PMEFlags::Write);
    pd->entryFromAddress(va2).set(PhysicalAddress::Null, PMEFlags::PageSize, PMEFlags::Present, PMEFlags::Write);

    asm volatile(R"(
        movq %0, %%rax
        movq %%rax, %%cr3
        )"
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