#include <stl/tuple.h>
#include <simo/interrupt.h>
#include <simo/gdt.h>
#include <simo/utils.h>
#include <printf.h>

namespace interrupts
{

struct InterruptContext
{
    uint64_t ip;
    uint64_t cs;
    uint64_t flags;
    uint64_t sp;
    uint64_t ss;
};

// TODO: how to ensure [[gnu::interrupt]]?
using InterruptHandler = void (*)(InterruptContext*);
using ExceptionHandler = void (*)(InterruptContext*, uint64_t);

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

    InterruptDescriptor() = default;

    InterruptDescriptor(InterruptHandler handler, gdt::Selector selector,
                        uint8_t stackTableOffset, uint8_t typeAndAttributes) :
        InterruptDescriptor(reinterpret_cast<uint64_t>(handler), selector, stackTableOffset, typeAndAttributes) {}

    InterruptDescriptor(ExceptionHandler handler, gdt::Selector selector,
                        uint8_t stackTableOffset, uint8_t typeAndAttributes) :
        InterruptDescriptor(reinterpret_cast<uint64_t>(handler), selector, stackTableOffset, typeAndAttributes) {}

private:
    static stl::Tuple<uint16_t, uint16_t, uint32_t> extractOffsets(uint64_t v)
    {
        auto offset1 = static_cast<uint16_t>(v & 0xfffful);
        auto offset2 = static_cast<uint16_t>((v >> 16) & 0xfffful);
        auto offset3 = static_cast<uint32_t>((v >> 32) & 0xffff'fffful);

        return { offset1, offset2, offset3 };
    }

    InterruptDescriptor(uint64_t handler, gdt::Selector selector,
                        uint8_t stackTableOffset, uint8_t typeAndAttributes)
    {
        auto [offset1, offset2, offset3] = extractOffsets(handler);
        
        this->offset1 = offset1;
        this->selector = static_cast<uint16_t>(selector);
        this->stackTableOffset = stackTableOffset;
        this->typeAndAttributes = typeAndAttributes;
        this->offset2 = offset2;
        this->offset3 = offset3;
        this->reserved = 0;
    }
};

InterruptDescriptor g_IDT[256] = {};

void dumpInterruptContext(const InterruptContext* ctx)
{
    printf(R"(context:
  rip:    %016lx
  cs:     %04lx
  flags:  %08lx
  rsp:    %016lx
  ss:     %04lx)""\n\n", ctx->ip, ctx->cs, ctx->flags, ctx->sp, ctx->ss);
}

[[gnu::interrupt]] void int3Handler(InterruptContext* ctx)
{
    printf("\n[int3]\n");
    dumpInterruptContext(ctx);
}

[[gnu::interrupt]] void pageFaultHandler(InterruptContext* ctx, uint64_t errorCode)
{
    uint64_t faultAddr;
    asm volatile("movq %[faultAddr], %%cr2" : [faultAddr]"=a"(faultAddr));

    printf("\n[omg pagefault]\n");
    printf("error:      %04lx\n", errorCode);
    printf("address:    %016lx\n", faultAddr);
    printf("present:    %s\n", (errorCode & 1) ? "yes" : "no");
    printf("access:     %s\n", (errorCode & 2) ? "write" : "read");

    dumpInterruptContext(ctx);

    asm volatile("hlt");
}

void init()
{
    g_IDT[3] = InterruptDescriptor(int3Handler, gdt::Selector::KernelCode, 0, 0b1000'0000 | 0b1111);
    g_IDT[0xE] = InterruptDescriptor(pageFaultHandler, gdt::Selector::KernelCode, 0, 0b1000'0000 | 0b1111);

    printf("loading IDT\n");

    struct [[gnu::packed]] {
        uint16_t size;
        void* offset;
    } idtr = {
        .size = sizeof(g_IDT) - 1,
        .offset = g_IDT
    };

    asm volatile(R"(
        lidt %[idtr]
        )"
        : : [idtr]"m"(idtr) //: "%rax"
    );
    
    printf("done\n");
}

}