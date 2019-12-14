#pragma once

#include <stddef.h>

constexpr size_t operator ""_KiB(unsigned long long v)
{
    return v * 1024;
}

constexpr size_t operator ""_MiB(unsigned long long v)
{
    return v * 1024 * 1024;
}

constexpr size_t operator ""_GiB(unsigned long long v)
{
    return v * 1024 * 1024 * 1024;
}
