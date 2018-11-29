#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

namespace memory
{

using PhysicalAddress = uint64_t;

void init();

}

#endif