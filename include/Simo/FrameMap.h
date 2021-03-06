#pragma once

#include <Simo/Kernel.h>
#include <Simo/Paging.h>
#include <Simo/Literals.h>
#include <STL/Tuple.h>

namespace paging
{

constexpr size_t PAGE_SIZE = 4_KiB;

class PhysicalFrameMap
{
public:
    PhysicalFrameMap(PhysicalAddress memoryBase, size_t memorySize);

    PhysicalAddress allocateFrame();
    PhysicalAddress getNextFreeFrame();
    void freeFrame(PhysicalAddress frame);
    void markFrame(PhysicalAddress address, bool used);
    bool isFrameUsed(PhysicalAddress address) const;
    size_t getBitmapSize() const;
    size_t getByteSize() const;

private:
    PhysicalAddress m_memoryBase;
    size_t m_bitmapSize;
    uint8_t m_bitmap[0];
};

}