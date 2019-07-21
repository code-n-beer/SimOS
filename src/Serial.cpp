#include <Simo/Kernel.h>
#include <Simo/Utils.h>
#include <STL/Bit.h>
#include <stdint.h>

namespace serial
{

const uint16_t COM1 = 0x3F8;

enum SerialPortOffsets : uint16_t
{
    Data = 0,
    DivisorLatchLow = 0,
    DivisorLatchHigh = 1,
    InterruptEnable = 1,
    FifoControl = 2,
    LineControl = 3,
    LineStatus = 5,
};

bool canTransmit()
{
    auto status = inb(COM1 + LineStatus);
    return (status & stl::bit(5)) != 0;
}

void write(char value)
{
    while (!canTransmit()) {}

    outb(COM1 + Data, static_cast<uint8_t>(value));
}

}