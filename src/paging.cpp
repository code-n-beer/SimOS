#include <simo/kernel.h>
#include <simo/multiboot.h>
#include <simo/paging.h>
#include <simo/pagemap.h>
#include <simo/utils.h>
#include <simo/framemap.h>
#include <printf.h>
#include <stl/tuple.h>
#include <stl/bit.h>

namespace paging
{

extern "C" char _kernelVirtualStart;
extern "C" char _kernelVirtualEnd;
extern "C" char _kernelPhysicalStart;
extern "C" char _kernelPhysicalEnd;
extern "C" char _kernelStackTopPA;
extern "C" char _kernelStackTopVA;
extern "C" char _kernelStackBottomVA;

using multiboot::MemoryType;
using multiboot::MmapEntry;
using multiboot::MmapTag;
using multiboot::TagType;
using multiboot::ElfSectionsTag;

constexpr PhysicalAddress alignToPage(PhysicalAddress addr)
{
    return PhysicalAddress{stl::align(PAGE_SIZE, static_cast<uint64_t>(addr))};
}

MmapEntry findBiggestMemoryArea(const MmapTag* mmap)
{
    MmapEntry biggest{};

    auto numEntries = (mmap->size - sizeof(MmapTag)) / mmap->entrySize;
    for (auto i = 0ul; i < numEntries; i++) {
        const auto& entry = mmap->entries[i];
        if (entry.type == MemoryType::Available && entry.len > biggest.len) {
            biggest = entry;
        }
    }

    return biggest;
}

PhysicalAddress identityMappedVirtualToPhysical(const void* addr)
{
    return PhysicalAddress{reinterpret_cast<uint64_t>(addr)};
}

template<typename T = void>
T* identityMappedPhysicalToVirtual(PhysicalAddress addr)
{
    return reinterpret_cast<T*>(static_cast<uint64_t>(addr));
}

// the name is pretty misleading, there's no guarantee that there's actual physical memory after
// the multiboot structure, but whatever
PhysicalAddress getFirstSafePhysicalAddress(const multiboot::Info* multibootInfo)
{
    auto physAddr = identityMappedVirtualToPhysical(multibootInfo);
    physAddr += multibootInfo->totalSize;

    return alignToPage(physAddr);
}

PhysicalFrameMap* initPhysicalFrameMap(const MmapTag* mmap, void* addr)
{
    const auto mem = findBiggestMemoryArea(mmap);
    printf("Physical memory at %016lx (size 0x%lx)\nbitmap at %p\n", mem.addr, mem.len, addr);
    return new (addr) PhysicalFrameMap(PhysicalAddress{mem.addr}, mem.len);
}

stl::Tuple<const ElfSectionsTag*, const MmapTag*> getMultibootTags(const multiboot::Info* multibootInfo)
{
    const ElfSectionsTag* elfSections = nullptr;
    const MmapTag* memoryMap = nullptr;

    for (const auto& tag : multibootInfo) {
        if (tag.type == TagType::ElfSections) {
            elfSections = static_cast<const ElfSectionsTag*>(&tag);
        } else if (tag.type == TagType::Mmap) {
            memoryMap = static_cast<const MmapTag*>(&tag);
        }
    }

    return {elfSections, memoryMap};
}

PhysicalFrameMap* g_physFrameMap = nullptr;

PML4& getPML4()
{
    return *reinterpret_cast<PML4*>(PML4::VirtualBaseAddress);
}

PDPT& getPDPT(const void* addr)
{
    return *reinterpret_cast<PDPT*>(
        PDPT::VirtualBaseAddress
        + PML4::indexFromAddress(addr) * 0x1000ul
    );
}

PD& getPD(const void* addr)
{
    return *reinterpret_cast<PD*>(
        PD::VirtualBaseAddress
        + PML4::indexFromAddress(addr) * 0x20'0000ul
        + PDPT::indexFromAddress(addr) * 0x1000ul
    );
}

PT& getPT(const void* addr)
{
    return *reinterpret_cast<PT*>(
        PT::VirtualBaseAddress
        + PML4::indexFromAddress(addr) * 0x4000'0000ul
        + PDPT::indexFromAddress(addr) * 0x20'0000ul
        + PD::indexFromAddress(addr) * 0x1000ul
    );
}

void initPDPTForAddress(PML4E* entry, const void* addr)
{
    auto pdptPA = g_physFrameMap->allocateFrame();
    entry->set(pdptPA, PMEFlags::Present | PMEFlags::Write);
    //printf("creating new PDPT at %016lx (va %p)\n", uint64_t(pdptPA), virtualAddr);

    new (&getPDPT(addr)) PDPT();
}

void initPDForAddress(PDPTE* entry, const void* addr)
{
    auto pdPA = g_physFrameMap->allocateFrame();
    entry->set(pdPA, PMEFlags::Present | PMEFlags::Write);
    //printf("creating new PD at %016lx (va %p)\n", uint64_t(pdPA), virtualAddr);

    new (&getPD(addr)) PD();
}

void initPTForAddress(PDE* entry, const void* addr)
{
    auto ptPA = g_physFrameMap->allocateFrame();
    entry->set(ptPA, PMEFlags::Present | PMEFlags::Write);
    //printf("creating new PT at %016lx (va %p)\n", uint64_t(ptPA), virtualAddr);

    new (&getPT(addr)) PT();
}

void mapPage(void* virtualAddr, PhysicalAddress physAddr, stl::Flags<PMEFlags> flags)
{
    if (auto& entry = getPML4().entryFromAddress(virtualAddr); !entry.isPresent()) {
        initPDPTForAddress(&entry, virtualAddr);
    }

    if (auto& entry = getPDPT(virtualAddr).entryFromAddress(virtualAddr); !entry.isPresent()) {
        initPDForAddress(&entry, virtualAddr);
    }

    if (auto& entry = getPD(virtualAddr).entryFromAddress(virtualAddr); !entry.isPresent()) {
        initPTForAddress(&entry, virtualAddr);
    }

    g_physFrameMap->markFrame(physAddr, true); // maybe check if it's already marked?
    getPT(virtualAddr).entryFromAddress(virtualAddr).set(physAddr, flags);
}

void mapRange(void* virtualAddr, PhysicalAddress physAddr, size_t length, stl::Flags<PMEFlags> flags)
{
    auto va = static_cast<char*>(virtualAddr);

    for (uint64_t mapped = 0; mapped < length; mapped += PAGE_SIZE) {
        mapPage(va, physAddr, flags);

        va += PAGE_SIZE;
        physAddr += PAGE_SIZE;
    }
}

void setupPageTables(const multiboot::Info* multibootInfo)
{
    auto [elfSections, memoryMap] = getMultibootTags(multibootInfo);
    // TODO: assert(elfSections && memoryMap);

    auto physFrameMapPA = getFirstSafePhysicalAddress(multibootInfo);
    // virtual address is physical + 0xffffffff80000000 at this point
    auto physFrameMapVA = identityMappedPhysicalToVirtual(physFrameMapPA + 0xffff'ffff'8000'0000ull);

    g_physFrameMap = initPhysicalFrameMap(memoryMap, physFrameMapVA);
    const auto physFrameMapEndPA = alignToPage(physFrameMapPA + g_physFrameMap->getByteSize());

    const auto startPtr = identityMappedVirtualToPhysical(&_kernelPhysicalStart);
    const auto endPtr = identityMappedVirtualToPhysical(&_kernelPhysicalEnd);

    // reserve the physical memory where the kernel was loaded
    printf("Reserving %016lx-%016lx...\n", uint64_t(startPtr), uint64_t(endPtr));
    for (auto ptr = startPtr; ptr < endPtr; ptr += 0x1000) {
        g_physFrameMap->markFrame(ptr, true);
    }

    // reserve the physical memory used by the frame allocator
    printf("Reserving %016lx-%016lx...\n", uint64_t(physFrameMapPA), uint64_t(physFrameMapEndPA));
    for (auto ptr = physFrameMapPA; ptr < physFrameMapEndPA; ptr += 0x1000) {
        g_physFrameMap->markFrame(ptr, true);
    }

    auto pml4PA = g_physFrameMap->allocateFrame();
    printf("PML4 is at %016lx\n", uint64_t(pml4PA));

    // Recursively map the new PML4
    auto pml4 = new (identityMappedPhysicalToVirtual(pml4PA)) PML4();
    pml4->entries[510].set(pml4PA, PMEFlags::Present | PMEFlags::Write);

    // Map the recursive entry of the PML4 to our new PML4
    getPML4().entries[510].set(pml4PA, PMEFlags::Present | PMEFlags::Write);

    // invalidate the TLB cache for the old PML4 recursive mapping
    asm volatile(R"(
        movq %0, %%rax
        invlpg (%%rax)
        )"
        : : "r"(pml4PA) : "%rax"
    );

    // todo: clean up
    auto kernelPA = identityMappedVirtualToPhysical(&_kernelPhysicalStart);

    // map the physical frame map
    mapRange(physFrameMapVA, physFrameMapPA, g_physFrameMap->getByteSize(), PMEFlags::Present | PMEFlags::Write);

    // map the kernel itself, TODO: map the sections properly
    auto kernelSize = &_kernelVirtualEnd - &_kernelVirtualStart;
    mapRange(&_kernelVirtualStart, kernelPA, kernelSize, PMEFlags::Present | PMEFlags::Write);

    // map the stack
    mapRange(&_kernelStackTopVA, identityMappedVirtualToPhysical(&_kernelStackTopPA), 0x4000, PMEFlags::Present | PMEFlags::Write);

    // map VGA console - TODO: rework the console itself
    mapPage((void*)0xb8000, PhysicalAddress{0xb8000}, PMEFlags::Present | PMEFlags::Write);

    asm volatile(R"(
        movq %0, %%rax
        movq %%rax, %%cr3
        )"
        : : "r"(pml4PA) : "%rax"
    );

    printf("No longer running with identity mapping \\:D/\n");
}

void init(const multiboot::Info* multibootInfo)
{
    setupPageTables(multibootInfo);
    printf("if you're reading this, memory mapping actually work\n");
}

}