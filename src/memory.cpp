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

template<typename TEntry>
struct PageMapBase
{
    static_assert(sizeof(TEntry) == sizeof(uint64_t), "Page maps must have 8-byte entries!");

    TEntry entries[512];
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

struct PDPT : public PageMapBase<PDPTE> {};
struct PD : public PageMapBase<PDE> {};
struct PT : public PageMapBase<PTE> {};
struct PML4 : public PageMapBase<PML4E> {};

extern "C" uint8_t _kernelVirtualStart;
extern "C" uint8_t _kernelVirtualEnd;
extern "C" uint8_t _kernelPhysicalStart;
extern "C" uint8_t _kernelPhysicalEnd;

void init(const MultibootBasicInfo* multibootInfo)
{
    auto pml4 = reinterpret_cast<PML4*>(0xffff'ffff'ffff'f000ull);
    const auto& entry = pml4->entries[0];
    printf("%016llx\n", PML4E::PHYS_ADDR_MASK);
    printf("pml4[0]: %016llx\n", entry.raw);
    printf("    present: %s\n", entry.isPresent() ? "yes" : "no");
    printf("    addr:    %016llx\n", entry.getPhysicalAddress());
    printf("virt start: %p\n", &_kernelVirtualStart);
    printf("virt end:   %p\n", &_kernelVirtualEnd);
    printf("phys start: %p\n", &_kernelPhysicalStart);
    printf("phys end:   %p\n", &_kernelPhysicalEnd);
}

}