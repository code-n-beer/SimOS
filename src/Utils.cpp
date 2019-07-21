#include <Simo/Utils.h>
#include <printf.h>
#include <Simo/Serial.h>

extern "C" void* memcpy(void* dest, const void* src, size_t length)
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

extern "C" void* memmove(void* dest, const void* src, size_t length)
{
    return memcpy(dest, src, length);
}

extern "C" void* memset(void* dest, int value, size_t length)
{
    auto destPtr = static_cast<char*>(dest);

    while (length--) {
        *destPtr++ = static_cast<char>(value);
    }

    return dest;
}

namespace assertion
{

constexpr const char* ASSERTION_FORMAT =
R"(
Assertion failed: %s
    at %s:%d:%s
)";

[[noreturn]] void assertionFailed(const char* msg, const char* file, int line, const char* func)
{
    printf(ASSERTION_FORMAT, msg, file, line, func);
    while (true) {
        asm("hlt");
    }
}

}

static PutcharHandler g_putchar = &serial::write;

void setPutcharHandler(PutcharHandler handler)
{
    g_putchar = handler;
}

extern "C" void _putchar(char c)
{
    if (!g_putchar) {
        // hmm
        return;
    }

    g_putchar(c);
}