#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

struct MultibootBasicInfo;

namespace memory
{

using PhysicalAddress = uint64_t;

void init(const MultibootBasicInfo*);

}

#endif