#include <Simo/Kernel.h>
#include <Simo/Paging.h>
#include <Simo/Literals.h>
#include <STL/Tuple.h>

namespace paging
{

using namespace literals;
const size_t PAGE_SIZE = 4KiB;

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
    stl::Tuple<size_t, uint64_t> computeEntryIndexAndMask(PhysicalAddress address) const;
    PhysicalAddress m_memoryBase;
    size_t m_bitmapSize;
    uint64_t m_bitmap[0];
};

}