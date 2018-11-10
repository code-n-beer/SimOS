#ifndef CONSOLE_H
#define CONSOLE_H

#include <stddef.h>
#include <stdint.h>

namespace console
{

enum class Color : uint8_t
{
    Black           = 0,
    Blue            = 1,
    Green           = 2,
    Cyan            = 3,
    Red             = 4,
    Magenta         = 5,
    Brown           = 6,
    LightGrey       = 7,
    DarkGrey        = 8,
    LightBlue       = 9,
    LightGreen      = 10,
    LightCyan       = 11,
    LightRed        = 12,
    LightMagenta    = 13,
    LightBrown      = 14,
    White           = 15,
};

void init();
void clear();

void setForegroundColor(Color c);
void setBackgroundColor(Color c);

void setPosition(size_t x, size_t y);
void putChar(char c);

}

#endif