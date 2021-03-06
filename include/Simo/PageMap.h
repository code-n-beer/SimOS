#pragma once

#include <stdint.h>
#include <stddef.h>

#include <Simo/Paging.h>
#include <STL/Bit.h>
#include <STL/Flags.h>
#include <STL/TypeTraits.h>

namespace paging
{

template<
    uint64_t PhysAddrMask,
    uint64_t PhysAddrMaskPage,
    PMEFlags... DisabledFlags>
struct PageMapEntry
{
    uint64_t raw;

    constexpr PageMapEntry() = default;
    constexpr PageMapEntry(const PageMapEntry&) = default;

    constexpr PageMapEntry(PhysicalAddress address, stl::Flags<PMEFlags> flags)
    {
        setFlags(flags);
        setPhysicalAddress(address);
    }

    constexpr bool hasFlags(stl::Flags<PMEFlags> flags) const
    {
        flags &= ~disabledFlags;
        return (raw & flags) != 0;
    }

    constexpr void setFlags(stl::Flags<PMEFlags> flags)
    {
        flags &= ~disabledFlags;
        raw |= flags.value();
    }

    constexpr PhysicalAddress getPhysicalAddress() const
    {
        uint64_t mask = PhysAddrMask;

        if constexpr(canMapPage) {
            if (hasFlags(PMEFlags::PageSize)) {
                mask = PhysAddrMaskPage;
            }
        }

        return PhysicalAddress{raw & mask};
    }

    constexpr void setPhysicalAddress(PhysicalAddress address)
    {
        uint64_t mask = PhysAddrMask;

        if constexpr(canMapPage) {
            if (hasFlags(PMEFlags::PageSize)) {
                mask = PhysAddrMaskPage;
            }
        }

        raw = (raw & ~mask) | (static_cast<uint64_t>(address) & mask);
    }

    constexpr void set(PhysicalAddress addr, stl::Flags<PMEFlags> flags)
    {
        setFlags(flags);
        setPhysicalAddress(addr);
    }

    constexpr bool isPresent() const
    {
        return raw & stl::Flags{PMEFlags::Present};
    }

private:
    static constexpr stl::Flags<PMEFlags> disabledFlags{DisabledFlags...};
    static constexpr bool canMapPage = !(disabledFlags & PMEFlags::PageSize);
};

// Intel docs says MAXPHYADDR is at most 52 on current platforms and I'd rather not deal with CPUID now
// so I'll just pretend it's always 52
const size_t MAXPHYADDR = 52;

using PML4E = PageMapEntry<
    stl::bitmask(MAXPHYADDR - 1, 12),   // PhysAddrMask
    0,                                  // PhysAddrMaskPage
    PMEFlags::Dirty, PMEFlags::PageSize // DisabledFlags
>;

using PDPTE = PageMapEntry<
    stl::bitmask(MAXPHYADDR - 1, 12),   // PhysAddrMask
    stl::bitmask(MAXPHYADDR - 1, 30)    // PhysAddrMaskPage
>;

using PDE = PageMapEntry<
    stl::bitmask(MAXPHYADDR - 1, 12),   // PhysAddrMask
    stl::bitmask(MAXPHYADDR - 1, 21)    // PhysAddrMaskPage
>;

using PTE = PageMapEntry<
    stl::bitmask(MAXPHYADDR - 1, 12),   // PhysAddrMask
    stl::bitmask(MAXPHYADDR - 1, 12),   // PhysAddrMaskPage
    PMEFlags::PageSize                  // DisabledFlags
>;

template<typename T>
constexpr bool ValidPageMapEntry =
    stl::IsTrivial<T>
    && stl::IsStandardLayout<T>
    && sizeof(T) == sizeof(uint64_t);

template<size_t S>
constexpr bool ValidAddrShift = (S >= 12) && (S <= 39);

template<typename TEntry, size_t VirtAddrShift, uint64_t BaseAddress>
struct PageMapBase
{
    static_assert(ValidAddrShift<VirtAddrShift>);
    static_assert(ValidPageMapEntry<TEntry>);

    TEntry entries[512];

    PageMapBase() :
        entries{{}}
    {}

    static uint16_t indexFromAddress(const void* addr)
    {
        return static_cast<uint16_t>(
            (reinterpret_cast<uint64_t>(addr) >> VirtAddrShift) & 0x1FF
        );
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

    TEntry& operator[](const void* addr)
    {
        return entryFromAddress(addr);
    }

    const TEntry& operator[](const void* addr) const
    {
        return entryFromAddress(addr);
    }

    static const uint64_t VirtualBaseAddress = BaseAddress;
};

struct PML4 : public PageMapBase<PML4E, 39, 0xffff'ff7f'bfdf'e000> {};
struct PDPT : public PageMapBase<PDPTE, 30, 0xffff'ff7f'bfc0'0000> {};
struct PD   : public PageMapBase<PDE,   21, 0xffff'ff7f'8000'0000> {};
struct PT   : public PageMapBase<PTE,   12, 0xffff'ff00'0000'0000> {};

}