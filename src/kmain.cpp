#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <multiboot2.h>
#include <printf.h>
#include <console.h>

#define PML4_IDX_FROM_ADDR(addr)  (((addr) >> 39) & 0x1FF)
#define PDPT_IDX_FROM_ADDR(addr)  (((addr) >> 30) & 0x1FF)
#define PD_IDX_FROM_ADDR(addr)    (((addr) >> 21) & 0x1FF)

extern uint64_t g_PML4[512];
extern uint64_t g_highPDPT[512];
extern uint64_t g_highPD[512];
extern uint64_t g_lowPDPT[512];
extern uint64_t g_lowPD[512];

extern "C" void kmain(uint64_t* multibootHeader)
{
    console::init();

    uint64_t* virtualPML4 = (uint64_t*)(0xFFFFFFFFFFE00000ULL);
    auto p = 0xFFFFFFFF80400000ULL;
    auto q0 = PML4_IDX_FROM_ADDR(p);
    auto q1 = PDPT_IDX_FROM_ADDR(p);
    auto q2 = PD_IDX_FROM_ADDR(p);
    
    printf("ptr:\t%016llx\n", p);
    printf("pml4:\t%016llx\n", q0);
    printf("pdpt:\t%016llx\n", q1);
    printf("pd:\t%016llx\n", q2);
    printf("\n\n");

    printf("kmain is at:\t%p\n", kmain);
    printf("PML4 is at:\t%p (%p virtual)\n", g_PML4, virtualPML4);
    printf("PML4[0] is:\t%016llx\n", g_PML4[0]);
    printf("vPML4[0] is:\t%016llx\n", virtualPML4[0]);
    printf("PML4[511] is:\t%016llx\n", g_PML4[511]);
    
    while (true) {
    }
}