#pragma once

#include <stdint.h>
#include <stddef.h>

#include <simo/memory.h>
#include <stl/bit.h>

// TODO: move these somewhere else

template<typename T, typename U>
constexpr bool hasBit(T value, U bits)
{
    return (value & static_cast<T>(bits)) != 0;
}

namespace memory
{

using stl::bit;
using stl::bitmask;

// Intel docs says MAXPHYADDR is at most 52 on current platforms and I'd rather not deal with CPUID now
// so I'll just pretend it's always 52
const size_t MAXPHYADDR = 52;

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

template<typename T>
concept bool ValidPageMapEntry = requires {
    sizeof(T) == 8;
    { T::PHYS_ADDR_MASK } -> uint64_t;
    typename T::Flags;
};

template<typename TEntry, size_t VirtAddrShift> requires ValidPageMapEntry<TEntry>
struct PageMapBase
{
    static_assert(sizeof(TEntry) == sizeof(uint64_t), "Page maps must have 8-byte entries!");
    static_assert(VirtAddrShift >= 12);
    static_assert(__is_trivial(TEntry));
    static_assert(__is_standard_layout(TEntry));

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

}