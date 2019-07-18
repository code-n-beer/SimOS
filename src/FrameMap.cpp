#include <Simo/FrameMap.h>
#include <Simo/Utils.h>
#include <printf.h>

namespace paging
{

PhysicalFrameMap::PhysicalFrameMap(PhysicalAddress memoryBase, size_t memorySize) :
    m_memoryBase(memoryBase), m_bitmapSize(memorySize / PAGE_SIZE)
{
    memset(m_bitmap, 0, m_bitmapSize);
}

PhysicalAddress PhysicalFrameMap::allocateFrame()
{
    if (auto frame = getNextFreeFrame(); frame != PhysicalAddress::Null) {
        markFrame(frame, true);
        return frame;
    }

    return PhysicalAddress::Null;
}

void PhysicalFrameMap::freeFrame(PhysicalAddress frame)
{
    // TODO: bounds checking
    markFrame(frame, false);
}

PhysicalAddress PhysicalFrameMap::getNextFreeFrame()
{
    auto addr = m_memoryBase;

    // TODO: make this not stupid
    while (true) {
        if (!isFrameUsed(addr)) {
            return addr;
        }

        addr += PAGE_SIZE;
    }

    return PhysicalAddress::Null;
}

void PhysicalFrameMap::markFrame(PhysicalAddress address, bool used)
{
    if (address < m_memoryBase) {
        printf("FIXME: %s:%d: can't mark frames below m_memoryBase!\n        (%016lx < %016lx)\n", __FILE__, __LINE__,
            uint64_t(address), uint64_t(m_memoryBase));
        return;
    }

    auto entryIdx = (address - m_memoryBase) / PAGE_SIZE;
    ASSERT(entryIdx < m_bitmapSize);

    auto& entry = m_bitmap[entryIdx];

    if (used) {
        // TODO: check for wraparound
        entry++;
    } else {
        ASSERT(entry > 0);
        entry--;
    }

    //printf("marked frame at %016lx: %d\n", uint64_t(address), entry);
}

bool PhysicalFrameMap::isFrameUsed(PhysicalAddress address) const
{
    auto entryIdx = (address - m_memoryBase) / PAGE_SIZE;
    ASSERT(entryIdx < m_bitmapSize);

    auto& entry = m_bitmap[entryIdx];

    return entry > 0;
}

size_t PhysicalFrameMap::getBitmapSize() const
{
    return m_bitmapSize;
}

size_t PhysicalFrameMap::getByteSize() const
{
    return sizeof(*this) + m_bitmapSize;
}

}