#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
 
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

void terminal_write(const char* data, size_t size) 
{
    for (size_t i = 0; i < size; i++)
        terminal_putchar(data[i]);
}
 
void terminal_writestring(const char* data) 
{
    terminal_write(data, strlen(data));
}

char nthdigit(int x, int n)
{
    while (n--) {
        x /= 10;
    }
    return (x % 10) + '0';
}
 
void print_num(int num)
{
    if(num < 10) {
        char* n = "1\n";
        n[0] = num + '0';
        return terminal_writestring(n);
    }

    int benis = num;
    unsigned int numDigits = 0;
    do {
        ++numDigits;
        benis /= 10;
    } while(benis);
    if(numDigits > 2) 
    {
        return terminal_writestring("over 2 digit numbers not supported :D kthx bye\n");
    }

    char* n = "00\n";
    n[0] = num / 10 + '0';
    n[1] = num % 10 + '0';
    terminal_writestring(n);
}
extern "C" void kmain(void) 
{
    /* Initialize terminal interface */
    terminal_initialize();
 
    for(int i = 0; i < 30; i++)
    {
        terminal_writestring("Hello, kernel World!\n");
        print_num(i);
    }
}

//extern "C" void kmain(const void* multibootHeader)
//{
//    unsigned short* vgaBase = reinterpret_cast<unsigned short*>(0xb8000);
//    const char ebin[] = "ebin";
//
//    for (auto line = 0; line < 10; line++) {
//        for (auto i = 0; ebin[i] != 0; i++) {
//            vgaBase[line * 80 + i] = 0x1f00 | ebin[i];
//        }
//    }
//
//    while (true);
//}
//