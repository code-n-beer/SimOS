#pragma once

#include <stdint.h>
#include <stl/bit.h>
#include <stl/flags.h>

namespace multiboot
{

struct Info;

}

namespace paging
{

enum class PMEFlags : uint64_t
{
    Present             = stl::bit(0),
    Write               = stl::bit(1),
    Supervisor          = stl::bit(2),
    PageWriteThrough    = stl::bit(3),
    PageCacheDisable    = stl::bit(4),
    Accessed            = stl::bit(5),
    Dirty               = stl::bit(6),
    PageSize            = stl::bit(7),
    ExecuteDisable      = stl::bit(63),
};

enum class PhysicalAddress : uint64_t
{
    Null = 0,
};

constexpr PhysicalAddress operator+(PhysicalAddress address, uint64_t offset)
{
    return PhysicalAddress{static_cast<uint64_t>(address) + offset};
}

constexpr PhysicalAddress operator-(PhysicalAddress address, uint64_t offset)
{
    return PhysicalAddress{static_cast<uint64_t>(address) - offset};
}

constexpr PhysicalAddress& operator+=(PhysicalAddress& address, uint64_t offset)
{
    address = address + offset;
    return address;
}

constexpr PhysicalAddress& operator-=(PhysicalAddress& address, uint64_t offset)
{
    address = address - offset;
    return address;
}

constexpr bool operator<(PhysicalAddress a, PhysicalAddress b)
{
    return static_cast<uint64_t>(a) < static_cast<uint64_t>(b);
}

void init(const multiboot::Info*);
void mapRange(void* virtualAddr, PhysicalAddress physAddr, size_t length, stl::Flags<PMEFlags> flags);

}
