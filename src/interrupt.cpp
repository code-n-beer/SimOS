#include <stl/tuple.h>
#include <simo/interrupt.h>
#include <simo/utils.h>
#include <printf.h>

namespace interrupts
{

struct interrupt_frame
{
    uint64_t ip;
    uint64_t cs;
    uint64_t flags;
    uint64_t sp;
    uint64_t ss;
};

// TODO: how to ensure [[gnu::interrupt]]?
using InterruptHandler = void (*)(interrupt_frame*);
using ExceptionHandler = void (*)(interrupt_frame*, uint64_t);

// TODO: make exception and interrupt handler descriptors separate types
struct [[gnu::packed]] InterruptDescriptor
{
    uint16_t offset1;
    uint16_t selector;
    uint8_t stackTableOffset;
    uint8_t typeAndAttributes;
    uint16_t offset2;
    uint32_t offset3;
    uint32_t reserved;

    static stl::Tuple<uint16_t, uint16_t, uint32_t> extractOffsets(uint64_t v)
    {
        auto offset1 = static_cast<uint16_t>(v & 0xfffful);
        auto offset2 = static_cast<uint16_t>((v >> 16) & 0xfffful);
        auto offset3 = static_cast<uint32_t>((v >> 32) & 0xffff'fffful);

        return { offset1, offset2, offset3 };
    }

    static InterruptDescriptor create(InterruptHandler handler, uint16_t selector,
                                      uint8_t stackTableOffset, uint8_t typeAndAttributes)
    {
        return create(reinterpret_cast<uint64_t>(handler), selector, stackTableOffset, typeAndAttributes);
    }

    static InterruptDescriptor create(ExceptionHandler handler, uint16_t selector,
                                      uint8_t stackTableOffset, uint8_t typeAndAttributes)
    {
        return create(reinterpret_cast<uint64_t>(handler), selector, stackTableOffset, typeAndAttributes);
    }

private:
    static InterruptDescriptor create(uint64_t handler, uint16_t selector,
                                      uint8_t stackTableOffset, uint8_t typeAndAttributes)
    {
        auto [offset1, offset2, offset3] = extractOffsets(handler);
        
        return {
            .offset1 = offset1,
            .selector = selector,
            .stackTableOffset = stackTableOffset,
            .typeAndAttributes = typeAndAttributes,
            .offset2 = offset2,
            .offset3 = offset3,
            .reserved = 0
        };
    }
};

InterruptDescriptor g_IDT[256] = {};

[[gnu::interrupt]] void int3Handler(interrupt_frame* ctx/*, uint64_t errorCode*/)
{
    printf("hehebin from int3 (%p)\n", ctx);
    /*memset((void*)0xb8000, 0x22, 2*80*25);
    asm volatile("hlt");*/
}

void init()
{
    memset(&g_IDT, 0, sizeof(g_IDT));
    g_IDT[3] = InterruptDescriptor::create(int3Handler, 1 << 3, 0, 0b1000'0000 | 0b1111);

    printf("loading IDT\n");

    struct [[gnu::packed]] {
        uint16_t size;
        uint64_t offset;
    } idtInfo = {
        .size = sizeof(g_IDT) - 1,
        .offset = (uint64_t)g_IDT
    };

    asm volatile(R"(
        movq %0, %%rax
        lidt (%%rax)
        )"
        : : "r"(&idtInfo) : "%rax"
    );
    
    printf("done\n");
}

}