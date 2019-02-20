#include <simo/kernel.h>

namespace gdt
{

enum class Selector : uint16_t
{
    KernelCode = 0x08,
    KernelData = 0x10,
};

void init();

}