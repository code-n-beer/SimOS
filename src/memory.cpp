#include <simo/kernel.h>
#include <simo/multiboot.h>
#include <simo/memory.h>
#include <printf.h>

template<typename T = uint64_t>
constexpr T bit(size_t offset)
{
    return T(1) << offset;
}

template<typename T, typename U>
constexpr bool hasBit(T value, U bits)
{
    return (value & static_cast<T>(bits)) != 0;
}

namespace memory
{

constexpr uint64_t bitmask(size_t to, size_t from)
{
    uint64_t result = 0;

    while (to >= from) {
        result |= 1UL << to;
        to--;
    }

    return result;
}

// Intel docs says MAXPHYADDR is at most 52 on current platforms and I'd rather not deal with CPUID now
// so I'll just pretend it's always 52
const size_t MAXPHYADDR = 52;

struct PDPT;
struct PD;
struct PT;

struct PML4E
{
    uint64_t raw;

    static const uint64_t PHYS_ADDR_MASK = bitmask(MAXPHYADDR - 1, 12);

    enum Flags : uint64_t
    {
        Present             = bit(0),
        Write               = bit(1),
        Supervisor          = bit(2),
        PageWriteThrough    = bit(3),
        PageCacheDisable    = bit(4),
        Accessed            = bit(5),
        ExecuteDisable      = bit(63),
    };

    bool isPresent() const              { return hasBit(raw, Present); }
    bool isWritable() const             { return hasBit(raw, Write); }
    bool isAccessed() const             { return hasBit(raw, Accessed); }

    PhysicalAddress getPhysicalAddress() const
    {
        return raw & PHYS_ADDR_MASK;
    }
};

struct PDPTE
{
    uint64_t raw;

    static const uint64_t PHYS_ADDR_MASK = bitmask(MAXPHYADDR - 1, 12);
    static const uint64_t PHYS_ADDR_MASK_PAGE = bitmask(MAXPHYADDR - 1, 30);

    enum Flags : uint64_t
    {
        Present             = bit(0),
        Write               = bit(1),
        Supervisor          = bit(2),
        PageWriteThrough    = bit(3),
        PageCacheDisable    = bit(4),
        Accessed            = bit(5),
        Dirty               = bit(6),
        PageSize            = bit(7),
        ExecuteDisable      = bit(63),
    };

    bool isPresent() const              { return hasBit(raw, Present); }
    bool isWritable() const             { return hasBit(raw, Write); }
    bool isAccessed() const             { return hasBit(raw, Accessed); }
    bool isPage() const                 { return hasBit(raw, PageSize); }
    bool isDirty() const                { return hasBit(raw, Dirty); }

    PhysicalAddress getPhysicalAddress() const
    {
        if (isPage()) {
            return raw & PHYS_ADDR_MASK_PAGE;
        }

        return raw & PHYS_ADDR_MASK;
    }
};

struct PDE
{
    uint64_t raw;

    static const uint64_t PHYS_ADDR_MASK = bitmask(MAXPHYADDR - 1, 12);
    static const uint64_t PHYS_ADDR_MASK_PAGE = bitmask(MAXPHYADDR - 1, 21);

    enum Flags : uint64_t
    {
        Present             = bit(0),
        Write               = bit(1),
        Supervisor          = bit(2),
        PageWriteThrough    = bit(3),
        PageCacheDisable    = bit(4),
        Accessed            = bit(5),
        Dirty               = bit(6),
        PageSize            = bit(7),
        ExecuteDisable      = bit(63),
    };

    bool isPresent() const              { return hasBit(raw, Present); }
    bool isWritable() const             { return hasBit(raw, Write); }
    bool isAccessed() const             { return hasBit(raw, Accessed); }
    bool isPage() const                 { return hasBit(raw, PageSize); }
    bool isDirty() const                { return hasBit(raw, Dirty); }

    PhysicalAddress getPhysicalAddress() const
    {
        if (isPage()) {
            return raw & PHYS_ADDR_MASK_PAGE;
        }

        return raw & PHYS_ADDR_MASK;
    }
};

struct PTE
{
    uint64_t raw;

    static const uint64_t PHYS_ADDR_MASK = bitmask(MAXPHYADDR - 1, 12);

    enum Flags : uint64_t
    {
        Present             = bit(0),
        Write               = bit(1),
        Supervisor          = bit(2),
        PageWriteThrough    = bit(3),
        PageCacheDisable    = bit(4),
        Accessed            = bit(5),
        Dirty               = bit(6),
        ExecuteDisable      = bit(63),
    };

    bool isPresent() const              { return hasBit(raw, Present); }
    bool isWritable() const             { return hasBit(raw, Write); }
    bool isAccessed() const             { return hasBit(raw, Accessed); }
    bool isDirty() const                { return hasBit(raw, Dirty); }

    PhysicalAddress getPhysicalAddress() const
    {
        return raw & PHYS_ADDR_MASK;
    }
};

template<typename TEntry, size_t VirtAddrShift>
struct PageMapBase
{
    static_assert(sizeof(TEntry) == sizeof(uint64_t), "Page maps must have 8-byte entries!");
    static_assert(VirtAddrShift >= 12);

    TEntry entries[512];

    uint64_t indexFromAddress(const void* addr) const
    {
        return (reinterpret_cast<uint64_t>(addr) >> VirtAddrShift) & 0x1FF;
    }

    TEntry& entryFromAddress(const void* addr)
    {
        auto index = indexFromAddress(addr);
        return entries[index];
    }

    const TEntry& entryFromAddress(const void* addr) const
    {
        auto index = indexFromAddress(addr);
        return entries[index];
    }
};

struct PML4 : public PageMapBase<PML4E, 39> {};
struct PDPT : public PageMapBase<PDPTE, 30> {};
struct PD   : public PageMapBase<PDE,   21> {};
struct PT   : public PageMapBase<PTE,   12> {};

extern "C" uint8_t _kernelVirtualStart;
extern "C" uint8_t _kernelVirtualEnd;
extern "C" uint8_t _kernelPhysicalStart;
extern "C" uint8_t _kernelPhysicalEnd;
extern "C" uint8_t _bootEnd;
extern "C" uint8_t _stack_bottom;

class PhysicalPageMap
{
public:
    static const size_t PAGE_SIZE = 0x1000;

    PhysicalPageMap(PhysicalAddress memoryBase, size_t memorySize) :
        m_memoryBase(memoryBase), 
        m_bitmapSize((memorySize / PAGE_SIZE) / (sizeof(uint64_t) * 8))
    {
        // TODO: write memset
        for (uint64_t i = 0; i < m_bitmapSize; i++) {
            m_bitmap[i] = 0;
        }
    }

    PhysicalAddress getNextFreePage()
    {
        auto addr = m_memoryBase;

        // TODO: make this not stupid
        while (true) {
            if (!isPageUsed(addr)) {
                return addr;
            }

            addr += PAGE_SIZE;
        }

        return 0ull;
    }

    void markPage(PhysicalAddress address, bool used)
    {
        const auto pageIdx = ((address - m_memoryBase) / PAGE_SIZE);
        const auto entryIdx = pageIdx / (sizeof(uint64_t) * 8);
        const auto shift = pageIdx % (sizeof(uint64_t) * 8);
        const auto mask = 1UL << shift;

        // TODO: assert(entryIdx < m_bitmapSize);
        // TODO: what if there are multiple mappings to the same frame and only one of them is unmapped and freed?

        if (used) {
            m_bitmap[entryIdx] |= mask;
        } else {
            m_bitmap[entryIdx] &= ~mask;
        }
    }

    bool isPageUsed(PhysicalAddress address) const
    {
        const auto pageIdx = ((address - m_memoryBase) / PAGE_SIZE);
        const auto entryIdx = pageIdx / (sizeof(uint64_t) * 8);
        const auto shift = pageIdx % (sizeof(uint64_t) * 8);
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

    // TODO: constexpr PhysicalAddress alignToPage(PhysicalAddress);
    physAddr = (physAddr - 1) & ~0xFFFUL;

    return physAddr;
}

PhysicalPageMap* initPhysicalPageMap(const MmapTag* mmap, void* addr)
{
    const auto mem = findBiggestMemoryArea(mmap);
    printf("Physical memory at %016lx (size 0x%lx)\n", mem.addr, mem.len);
    return new (addr) PhysicalPageMap(mem.addr, mem.len);
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

PhysicalPageMap* g_physPageMap = nullptr;

void setupPageTables(const MultibootBasicInfo* multibootInfo)
{
    auto [elfSections, memoryMap] = getMultibootTags(multibootInfo);
    // TODO: assert(elfSections && memoryMap);

    auto physPageMapPA = getFirstSafePhysicalAddress(multibootInfo);
    // virtual address is physical + 0xffffffff80000000 at this point
    auto physPageMapVA = reinterpret_cast<void*>(physPageMapPA + 0xffff'ffff'8000'0000ull);

    g_physPageMap = initPhysicalPageMap(memoryMap, reinterpret_cast<void*>(physPageMapVA));
    const auto physPageMapEndPA = (((physPageMapPA + g_physPageMap->getByteSize()) - 1) & ~0xFFFUL) + 0x1000;

    auto startPtr = reinterpret_cast<PhysicalAddress>(&_kernelPhysicalStart);
    auto endPtr = reinterpret_cast<PhysicalAddress>(&_kernelPhysicalEnd);

    // reserve the physical memory where the kernel was loaded
    printf("Reserving %016lx-%016lx...\n", startPtr, endPtr);
    for (auto ptr = startPtr; ptr < endPtr; ptr += 0x1000) {
        g_physPageMap->markPage(reinterpret_cast<PhysicalAddress>(ptr), true);
    }

    // reserve the physical memory used by the frame allocator
    printf("Reserving %016lx-%016lx...\n", physPageMapPA, physPageMapEndPA);
    for (auto ptr = physPageMapPA; ptr < physPageMapEndPA; ptr += 0x1000) {
        g_physPageMap->markPage(ptr, true);
    }

    // TODO: getNextFreePage + markPage -> PhysicalPageMap::allocateFrame
    auto pml4PA = g_physPageMap->getNextFreePage();
    auto pml4 = new (reinterpret_cast<void*>(pml4PA)) PML4();
    g_physPageMap->markPage(pml4PA, true);
    printf("PML4 is at %016lx\n", pml4PA);

    auto pdptPA = g_physPageMap->getNextFreePage();
    auto pdpt = new (reinterpret_cast<void*>(pdptPA)) PDPT();
    printf("PDPT is at %016lx\n", pdptPA);
    g_physPageMap->markPage(pdptPA, true);

    auto pdpt2PA = g_physPageMap->getNextFreePage();
    auto pdpt2 = new (reinterpret_cast<void*>(pdpt2PA)) PDPT();
    printf("PDPT2 is at %016lx\n", pdpt2PA);
    g_physPageMap->markPage(pdpt2PA, true);

    auto pdPA = g_physPageMap->getNextFreePage();
    auto pd = new (reinterpret_cast<void*>(pdPA)) PD();
    printf("PD is at %016lx\n", pdPA);
    g_physPageMap->markPage(pdPA, true);

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
}

}