#pragma once

#include <stdint.h>
#include <stddef.h>

#include <simo/memory.h>
#include <stl/bit.h>
#include <stl/flags.h>
#include <stl/typetraits.h>

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

enum class PMEFlags : uint64_t
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

template<
    uint64_t PhysAddrMask,
    uint64_t PhysAddrMaskPage,
    PMEFlags... DisabledFlags>
struct PageMapEntry
{
    uint64_t raw;

    PageMapEntry() = default;
    PageMapEntry(const PageMapEntry&) = default;

    PageMapEntry(PhysicalAddress address, stl::Flags<PMEFlags> flags)
    {
        setFlags(flags);
        setPhysicalAddress(address);
    }

    bool hasFlags(stl::Flags<PMEFlags> flags) const
    {
        flags &= ~disabledFlags;
        return (raw & flags) != 0;
    }

    void setFlags(stl::Flags<PMEFlags> flags)
    {
        flags &= ~disabledFlags;
        raw |= flags.value();
    }

    PhysicalAddress getPhysicalAddress() const
    {
        uint64_t mask = PhysAddrMask;

        if constexpr(canMapPage) {
            if (hasFlags(PMEFlags::PageSize)) {
                mask = PhysAddrMaskPage;
            }
        }

        return raw & mask;
    }

    void setPhysicalAddress(PhysicalAddress address)
    {
        uint64_t mask = PhysAddrMask;

        if constexpr(canMapPage) {
            if (hasFlags(PMEFlags::PageSize)) {
                mask = PhysAddrMaskPage;
            }
        }

        raw = (raw & ~mask) | (address & mask);
    }

    template<typename... Ts>
    void set(PhysicalAddress addr, Ts... flags)
    {
        setFlags({flags...});
        setPhysicalAddress(addr);
    }

private:
    static constexpr stl::Flags<PMEFlags> disabledFlags{DisabledFlags...};
    static constexpr bool canMapPage = !(disabledFlags & PMEFlags::PageSize);
};

// Intel docs says MAXPHYADDR is at most 52 on current platforms and I'd rather not deal with CPUID now
// so I'll just pretend it's always 52
const size_t MAXPHYADDR = 52;

using PML4E = PageMapEntry<
    bitmask(MAXPHYADDR - 1, 12),        // PhysAddrMask
    0,                                  // PhysAddrMaskPage
    PMEFlags::Dirty, PMEFlags::PageSize // DisabledFlags
>;

using PDPTE = PageMapEntry<
    bitmask(MAXPHYADDR - 1, 12),        // PhysAddrMask
    bitmask(MAXPHYADDR - 1, 30)         // PhysAddrMaskPage
>;

using PDE = PageMapEntry<
    bitmask(MAXPHYADDR - 1, 12),        // PhysAddrMask
    bitmask(MAXPHYADDR - 1, 21)         // PhysAddrMaskPage
>;

using PTE = PageMapEntry<
    bitmask(MAXPHYADDR - 1, 12),        // PhysAddrMask
    bitmask(MAXPHYADDR - 1, 21),        // PhysAddrMaskPage
    PMEFlags::PageSize                  // DisabledFlags
>;

template<typename T>
concept bool ValidPageMapEntry =
    stl::concepts::IsTrivial<T>
    && stl::concepts::IsStandardLayout<T>
    && requires {
        sizeof(T) == sizeof(uint64_t);
    };

template<size_t S>
concept bool ValidAddrShift = (S >= 12) && (S <= 39);

template<ValidPageMapEntry TEntry, ValidAddrShift VirtAddrShift>
struct PageMapBase
{
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