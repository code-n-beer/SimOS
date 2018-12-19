#pragma once

#include <stddef.h>
#include <stdint.h>

namespace stl
{

template<typename T = uint64_t>
constexpr T bit(size_t offset)
{
    return T(1) << offset;
}

template<typename T = uint64_t>
constexpr T bitmask(size_t to, size_t from)
{
    T result = 0;

    while (to >= from) {
        result |= T(1) << to;
        to--;
    }

    return result;
}

}