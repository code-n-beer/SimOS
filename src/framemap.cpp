#include <simo/framemap.h>
#include <simo/utils.h>
#include <printf.h>

namespace paging
{

PhysicalFrameMap::PhysicalFrameMap(PhysicalAddress memoryBase, size_t memorySize) :
    m_memoryBase(memoryBase), m_bitmapSize(stl::align(64, memorySize / PAGE_SIZE) / 64)
{
    memset(m_bitmap, 0, m_bitmapSize * sizeof(uint64_t));
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

    auto [entry, mask] = computeEntryIndexAndMask(address);

    // TODO: assert(entryIdx < m_bitmapSize);
    // TODO: what if there are multiple mappings to the same frame and only one of them is unmapped and freed?

    if (used) {
        m_bitmap[entry] |= mask;
    } else {
        m_bitmap[entry] &= ~mask;
    }
}

bool PhysicalFrameMap::isFrameUsed(PhysicalAddress address) const
{
    auto [entry, mask] = computeEntryIndexAndMask(address);

    // TODO: assert(entryIdx < m_bitmapSize);

    return (m_bitmap[entry] & mask) != 0;
}

size_t PhysicalFrameMap::getBitmapSize() const
{
    return m_bitmapSize;
}

size_t PhysicalFrameMap::getByteSize() const
{
    return sizeof(*this) + m_bitmapSize * sizeof(uint64_t);
}

stl::Tuple<size_t, uint64_t> PhysicalFrameMap::computeEntryIndexAndMask(PhysicalAddress address) const
{
    const auto frameIdx = ((static_cast<uint64_t>(address) - static_cast<uint64_t>(m_memoryBase)) / PAGE_SIZE);
    const auto entryIdx = frameIdx / (sizeof(uint64_t) * 8);
    const auto shift = frameIdx % (sizeof(uint64_t) * 8);
    const auto mask = 1UL << shift;

    return {entryIdx, mask};
}

}