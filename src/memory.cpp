#include <kernel.h>
#include <multiboot.h>
#include <memory.h>
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

enum struct PdpteFlags : uint64_t
{
    Present             = bit(0),
    Write               = bit(1),
    Supervisor          = bit(2),
    PageWriteThrough    = bit(3),
    PageCacheDisable    = bit(4),
    Accessed            = bit(5),
    ExecuteDisable      = bit(63),
};

constexpr uint64_t bitmask(size_t to, size_t from)
{
    uint64_t result = 0;

    while (to >= from) {
        result |= 1ULL << to;
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

void setupPageTables(const MultibootBasicInfo* multibootInfo)
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

    // TODO: assert(elfSections && memoryMap);

    // at this point we still have the first 1GB identity mapped, so we can do whatever
    auto pml4 = new (&_kernelPhysicalEnd + sizeof(PML4) * 0) PML4();
    auto pdpt = new (&_kernelPhysicalEnd + sizeof(PML4) * 1) PDPT();
    auto pd = new (&_kernelPhysicalEnd + sizeof(PML4) * 2) PD();
    auto pt = new (&_kernelPhysicalEnd + sizeof(PML4) * 3) PT();
    auto pd2 = new (&_kernelPhysicalEnd + sizeof(PML4) * 4) PD();
    auto pdpt2 = new (&_kernelPhysicalEnd + sizeof(PML4) * 5) PDPT();

    // map kernel stuff, todo: map it properly from elf sections
    pml4->entryFromAddress(&_kernelVirtualStart) = { reinterpret_cast<uint64_t>(pdpt) | PML4E::Flags::Present | PML4E::Flags::Write };
    pdpt->entryFromAddress(&_kernelVirtualStart) = { reinterpret_cast<uint64_t>(pd) | PDPTE::Flags::Present | PDPTE::Flags::Write };
    pd->entryFromAddress(&_kernelVirtualStart) = { reinterpret_cast<uint64_t>(pt) | PDE::Flags::Present | PDE::Flags::Write };

    // identity map the first 2MB of physical memory so we can write to the VGA console
    pml4->entryFromAddress(0) = { reinterpret_cast<uint64_t>(pdpt2) | PML4E::Flags::Present | PML4E::Flags::Write };
    pdpt2->entries[0] = { reinterpret_cast<uint64_t>(pd2) | PDPTE::Flags::Present | PDPTE::Flags::Write };
    pd2->entries[0] = { uint64_t(0) | PDE::Flags::Present | PDE::Flags::Write | PDE::Flags::PageSize };

    printf("PML4\n");
    for (int i = 0; i < 512; i++) {
        if (pml4->entries[i].isPresent()) {
            printf("%d: %016llx\n", i, pml4->entries[i]);
        }
    }

    printf("\nPDPT\n");
    for (int i = 0; i < 512; i++) {
        if (pdpt->entries[i].isPresent()) {
            printf("%d: %016llx\n", i, pdpt->entries[i]);
        }
    }

    uint8_t* virtPtr = &_kernelVirtualStart;
    uint8_t* physPtr = &_bootEnd;

    printf("\nPT\n");
    // TODO: fuck
    while (virtPtr != &_kernelVirtualEnd) {
        pt->entryFromAddress(virtPtr) = { reinterpret_cast<uint64_t>(physPtr) | PTE::Flags::Present | PTE::Flags::Write };
        auto i = pt->indexFromAddress(virtPtr);
        printf("%lld: %016llx\n", i, pt->entries[i]);
        virtPtr += 0x1000;
        physPtr += 0x1000;
    }

    asm volatile(
        "movq %0, %%rax\n"
        "movq %%rax, %%cr3\n"
        : : "r"(pml4) : "%rax"
    );
}

void init(const MultibootBasicInfo* multibootInfo)
{
    setupPageTables(multibootInfo);
    printf("if we returned here it's all good\n");
}

}