#include <simo/kernel.h>
#include <simo/console.h>
#include <simo/utils.h>
#include <stl/array.h>

namespace
{

const size_t CONSOLE_WIDTH = 80;
const size_t CONSOLE_HEIGHT = 25;

size_t x;
size_t y;
console::Color background;
console::Color foreground;

using ConsoleBuffer = stl::Array<uint16_t, CONSOLE_WIDTH * CONSOLE_HEIGHT>;
ConsoleBuffer* vgaBuffer = nullptr;

}

namespace console
{

void init()
{
    vgaBuffer = new (reinterpret_cast<void*>(0xb8000)) ConsoleBuffer();
    setForegroundColor(Color::LightGrey);
    setBackgroundColor(Color::Black);
}

void clear()
{
    x = 0;
    y = 0;
    memset(vgaBuffer->data(), 0, vgaBuffer->byteSize());
}

void setForegroundColor(Color c)
{
    foreground = c;
}

void setBackgroundColor(Color c)
{
    background = c;
}

void setPosition(size_t newX, size_t newY)
{
    x = (newX < CONSOLE_WIDTH) ? newX : x;
    y = (newY < CONSOLE_HEIGHT) ? newY : y;
}

void scrollSingleLine()
{
    memmove(
        &vgaBuffer->at(0),
        &vgaBuffer->at(CONSOLE_WIDTH),
        CONSOLE_WIDTH * (CONSOLE_HEIGHT - 1) * sizeof(uint16_t)
    );

    memset(
        &vgaBuffer->at((CONSOLE_HEIGHT - 1) * CONSOLE_WIDTH),
        0,
        CONSOLE_WIDTH * sizeof(uint16_t)
    );
}

void moveByOffset(size_t xOff, size_t yOff)
{
    if (x + xOff >= CONSOLE_WIDTH) {
        x = 0;
        yOff++;
    } else {
        x += xOff;
    }

    if (yOff > 0) {
        x = 0;
    }

    if (y + yOff >= CONSOLE_HEIGHT) {
        y = CONSOLE_HEIGHT - 1;

        while (yOff-- > 0) {
            scrollSingleLine();
        }
    } else {
        y += yOff;
    }
}

void incrementPosition(bool column, bool row)
{
    if (!column && !row) {
        return;
    }

    if (!row) {
        x++;

        if (x >= CONSOLE_WIDTH) {
            x = 0;
            y++;
        }
    } else {
        x = 0;
        y++;
    }

    if (y >= CONSOLE_HEIGHT) {
        y = CONSOLE_HEIGHT - 1;
        scrollSingleLine();
    }
}

void putChar(char c)
{
    if (c == '\n') {
        //incrementPosition(false, true);
        moveByOffset(0, 1);
    } else if (c == '\t') {
        auto nextX = (x & ~7) + 8;
        if (nextX >= CONSOLE_WIDTH) {
            //incrementPosition(false, true);
            moveByOffset(0, 1);
        } else {
            x = nextX;
        }
    } else {
        uint16_t value = (uint16_t(background) << 12) | (uint16_t(foreground) << 8) | uint8_t(c);
        vgaBuffer->at(y * CONSOLE_WIDTH + x) = value;
        //incrementPosition(true, false);
        moveByOffset(1, 0);
    }
}

}

extern "C" void _putchar(char c)
{
    console::putChar(c);
}