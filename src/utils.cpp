#include <simo/utils.h>

void* memcpy(void* dest, const void* src, size_t length)
{
    if (dest == src || length == 0) {
        return dest;
    }

    auto destPtr = static_cast<char*>(dest);
    auto srcPtr = static_cast<const char*>(src);

    if (reinterpret_cast<uintptr_t>(dest) < reinterpret_cast<uintptr_t>(src)) {
        // Copy forward
        while (length--) {
            *destPtr++ = *srcPtr++;
        }
    } else {
        // Copy backwards
        destPtr += length;
        srcPtr += length;

        while (length--) {
            *--destPtr = *--srcPtr;
        }
    }

    return dest;
}

void* memmove(void* dest, const void* src, size_t length)
{
    return memcpy(dest, src, length);
}

void* memset(void* dest, int value, size_t length)
{
    auto destPtr = static_cast<char*>(dest);

    while (length--) {
        *destPtr++ = static_cast<char>(value);
    }

    return dest;
}