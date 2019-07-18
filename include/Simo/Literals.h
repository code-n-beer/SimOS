#pragma once

#include <stddef.h>

namespace literals
{

constexpr size_t operator""KiB(unsigned long long v)
{
    return v * 1024;
}

constexpr size_t operator""MiB(unsigned long long v)
{
    return v * 1024KiB;
}

constexpr size_t operator""GiB(unsigned long long v)
{
    return v * 1024MiB;
}

}
