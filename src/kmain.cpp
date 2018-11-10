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

    //uint64_t* virtualPML4 = (uint64_t*)(0xFFFFFFFFFFE00000ULL);
    uint64_t* virtualPML4 = (uint64_t*)(0xFFFFFFFFFFFFF000ULL);

    printf("PML4 dump:\n\n");
    for (int i = 0; i < 512; i++) {
        printf("PML4[%d]: %016llx\n", i, virtualPML4[i]);
    }
    
    while (true) {
    }
}