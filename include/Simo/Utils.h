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
