#pragma once

#include <stddef.h>
#include <stdint.h>

#define memcpy __builtin_memcpy
#define memset __builtin_memset
#define memmove __builtin_memmove

namespace assertion
{

[[noreturn]] void assertionFailed(const char* msg, const char* file, int line, const char* func);

inline void doAssert(bool expression, const char* msg, const char* file = __builtin_FILE(),
    int line = __builtin_LINE(), const char* func = __builtin_FUNCTION())
{
    if (!expression) {
        assertionFailed(msg, file, line, func);
    }
}

}

#define ASSERT(expr) assertion::doAssert((expr), #expr)

inline void outb(uint16_t port, uint8_t value)
{
    asm volatile("outb %0, %1" : : "a"(value), "d"(port));
}

inline uint8_t inb(uint16_t port)
{
    uint8_t value = 0;

    asm volatile("inb %1, %0" : "=a"(value) : "d"(port));

    return value;
}

using PutcharHandler = void (*)(char);

void setPutcharHandler(PutcharHandler handler);
