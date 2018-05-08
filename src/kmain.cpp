extern "C" void kmain(const void* multibootHeader)
{
    unsigned short* vgaBase = reinterpret_cast<unsigned short*>(0xb8000);
    const char ebin[] = "ebin";

for (auto line = 0; line < 10; line++) {
    for (auto i = 0; ebin[i] != 0; i++) {
        vgaBase[line * 80 + i] = 0x1f00 | ebin[i];
    }
}

    while (true);
}