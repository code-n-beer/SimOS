#pragma once

#include <stdint.h>

namespace multiboot
{

struct Info;

}

namespace memory
{

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

}
