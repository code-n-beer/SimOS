#pragma once

#include <stddef.h>
#include <stdint.h>

void* memcpy(void* dest, const void* src, size_t length);
void* memmove(void* dest, const void* src, size_t length);
void* memset(void* dest, int value, size_t length);

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
