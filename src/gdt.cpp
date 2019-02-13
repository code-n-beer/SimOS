#include <simo/gdt.h>
#include <stl/bit.h>

namespace gdt
{

enum SegmentDescriptorFlags : uint64_t
{
    LongMode = stl::bit(53),
    Present = stl::bit(47),
    CodeOrData = stl::bit(44),
    CodeSegment = stl::bit(43),
    DataWritable = stl::bit(41),
    CodeReadable = stl::bit(41),
};

uint64_t g_GDT[] = {
    0,  // null descriptor
    LongMode | Present | CodeOrData | CodeSegment | CodeReadable,   // kernel code segment
    LongMode | Present | CodeOrData | DataWritable,                 // kernel data segment
};

void init()
{
    struct [[gnu::packed]] {
        uint16_t limit;
        void* gdt;
    } gdtr = {
        .limit = sizeof(g_GDT) - 1,
        .gdt = g_GDT
    };

    asm volatile(R"(
            lgdt %[gdtr]
            pushw $0x8
            push $next
            .byte 0x48  # need to emit rex.W prefix manually
            ljmpl *(%%rsp)
        next:
            add $10, %%rsp
            mov $0x10, %%ax
            mov %%ax, %%ds
            mov %%ax, %%es
            mov %%ax, %%fs
            mov %%ax, %%gs
            mov %%ax, %%ss
        )" : : [gdtr]"m"(gdtr) : "%rax"); 
}

}