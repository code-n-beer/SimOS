#pragma once

#include <stdint.h>

struct MultibootBasicInfo;

namespace memory
{

using PhysicalAddress = uint64_t;

void init(const MultibootBasicInfo*);

}
