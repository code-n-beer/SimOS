#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <multiboot2.h>
#include <printf.h>

/* Check if the compiler thinks we are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif
 
/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__x86_64__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif
 
/* Hardware text mode color constants. */
enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};
 
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) 
{
    return fg | bg << 4;
}
 
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) 
{
    return (uint16_t) uc | (uint16_t) color << 8;
}
 
size_t strlen(const char* str) 
{
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}
 
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
 
size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
 
/* Note the use of the volatile keyword to prevent the compiler from eliminating dead stores. */
volatile uint16_t* terminal_buffer;
 
void terminal_initialize(void) 
{
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK);
    terminal_buffer = (uint16_t*) 0xB8000;
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
}
 
void terminal_setcolor(uint8_t color) 
{
    terminal_color = color;
}
 
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) 
{
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}
 
void terminal_scroll_line() {
    for(size_t row = 1; row < VGA_HEIGHT; row++) {
        for(size_t col = 0; col < VGA_WIDTH; col++) {
            const size_t index = row * VGA_WIDTH + col;
            terminal_putentryat(terminal_buffer[index], terminal_color, col, row - 1);

            if(row == VGA_HEIGHT - 1)
            {
                terminal_putentryat(' ', terminal_color, col, row);
            }
        }
    }
}

void terminal_putchar(char c) 
{
    if(c == '\n') {
        terminal_row++;
        terminal_column = 0;
        if (terminal_row == VGA_HEIGHT)
        {
            terminal_row = VGA_HEIGHT - 1;
            terminal_scroll_line();
        }
        return;
    }

    terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
    if (++terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT)
        {
            terminal_row = VGA_HEIGHT - 1;
            terminal_scroll_line();
        }
    }
}

void _putchar(char c)
{
    terminal_putchar(c);
}

template<typename T>
void print_bitmap(T value)
{
    uint32_t length = 8 * sizeof(T);
    char buf[length + 1] = { 0 };
    T temp = value;
    for(int i = 0; i < length; i++)
    {
        buf[length - 1 - i] = (((value >> i) & 1) ? 1 : 0) + '0';
    }
    printf("%s\n", buf);
}

extern uint64_t g_PML4[512];
extern uint64_t g_PDP[512];
extern uint64_t g_PD[512];

uint16_t p4idx = 0;
uint16_t p3idx = 0;
uint16_t p2idx = 0;
// run once for each found available space
void map_mem(uint64_t base_addr, uint64_t length)
{
    if(base_addr == 0)
    {
        // don't map beginning because it sounds like a bad idea idk y
        return;
    }
    
    uint64_t pageSize = (2 * 1024 * 1024); // 2MB
    uint64_t fittingPageCount = length / pageSize;
    for(uint64_t i = 0; i < fittingPageCount; i++)
    {
        // Wat do if idx does not yet exist?
        // if(!g_PDP3[p3idx])
        // {
        //     g_PDP3[p3idx] = new g_PD jotainjotain ...
        // }

        // y u no work
        //g_PML4[p4idx][p3idx][p2idx] = base_addr + i * pageSize;

        p2idx++;
        if(p2idx >= 511)
         {
            p3idx++;
            p2idx = 0;
        }
        if(p3idx >= 511)
         {
            p4idx++;
            p3idx = 0;
        }
    }
}

extern "C" void kmain(uint64_t* multibootHeader)
{
    /* Initialize terminal interface */
    terminal_initialize();

    printf("\nPML4\n");
    for (int i = 0; i < 8; i++) {
        printf("%llx\n", g_PML4[i]);
    }

    printf("\nPDP\n");
    for (int i = 0; i < 8; i++) {
        printf("%llx\n", g_PDP[i]);
    }

    printf("\nPD\n");
    for (int i = 0; i < 8; i++) {
        printf("%llx\n", g_PD[i]);
    }

    printf("\n multiboot header flags\n");
    // 0 is flags
    print_bitmap(multibootHeader[0]);


    typedef struct tag
    {
        uint32_t type;
        uint32_t size;
    } tag;

    typedef struct multiboot_mmap_entry
    {
        uint64_t base_addr;
        uint64_t length;
        uint32_t type;
        uint32_t reserved_zero;
    } multiboot_mmap_entry;

    uint32_t* total_size = reinterpret_cast<uint32_t*>(multibootHeader);
    printf("\n total size of tags structure %llx\n", *total_size);
    printf("\n reserved %llx\n", *(total_size + 1));

    // tags start from second idx, first is u32 total size of tag structure + u32 reserved 
    uint64_t* tagp = multibootHeader + 1;

    while(tagp < multibootHeader + (*total_size) / sizeof(multibootHeader))
    {
        tag* t = reinterpret_cast<tag*>(tagp);

        // pad size to 8 bytes
        uint32_t size_bytes = ((t->size - 1) & ~0x7) + 0x8;

        if(t->type == 0x6) {
            printf("tag type %d\n", t->type);
            printf("tag size %d\n", t->size);

            uint32_t* entry_size = reinterpret_cast<uint32_t*>(t+1);
            uint32_t* entry_version = entry_size + 1;
            printf("entry_size and version: %d %d\n", *entry_size, *entry_version);

            // entries start after entry_version
            multiboot_mmap_entry* entry = reinterpret_cast<multiboot_mmap_entry*>(entry_version + 1);

            int available = 0;
            int reserved = 0;
            int acpi_reclaimable = 0;
            int nvs = 0;
            int badram = 0;
            for(int i = 0; i < t->size / (*entry_size); i++)
            {
                if (entry->type == MULTIBOOT_MEMORY_AVAILABLE)
                {
                    printf("base_addr %llx, length %llx\n", entry->base_addr, entry->length);
                }
                //terminal_writestring("\n type \n");
                //print_hex(entry->type);
                //terminal_writestring("\n reserved zeroes \n");
                //print_hex(entry->reserved_zero);

                switch(entry->type)
                {
                    case MULTIBOOT_MEMORY_AVAILABLE: available++; break;
                    case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE: acpi_reclaimable++; break;
                    case MULTIBOOT_MEMORY_BADRAM: badram++; break;
                    case MULTIBOOT_MEMORY_NVS: nvs++; break;
                    case MULTIBOOT_MEMORY_RESERVED: reserved++; break;
                }

                entry++;
            }

            //terminal_writestring("\n available \n");
            //print_num(available);
            //terminal_writestring("\n reserved \n");
            //print_num(reserved);
            //terminal_writestring("\n acpi_reclaimable \n");
            //print_num(acpi_reclaimable);
            //terminal_writestring("\n nvs \n");
            //print_num(nvs);
            //terminal_writestring("\n badram \n");
            //print_num(badram);
            
            //terminal_writestring("\n tag size padded \n");
            //print_hex(size_bytes);
        }

        tagp = tagp + size_bytes / sizeof(uint64_t);
    }
}