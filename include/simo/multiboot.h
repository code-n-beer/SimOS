#pragma once

#include <stdint.h>
#include <stl/bit.h>

namespace multiboot
{

enum class TagType : uint32_t
{
    End = 0,
    Cmdline = 1,
    BootLoaderName = 2,
    Module = 3,
    BasicMeminfo = 4,
    Bootdev = 5,
    Mmap = 6,
    Vbe = 7,
    Framebuffer = 8,
    ElfSections = 9,
    Apm = 10,
    Efi32 = 11,
    Efi64 = 12,
    Smbios = 13,
    AcpiOld = 14,
    AcpiNew = 15,
    Network = 16,
    EfiMmap = 17,
    EfiBs = 18,
    Efi32Ih = 19,
    Efi64Ih = 20,
    LoadBaseAddr = 21,
};

enum class MemoryType : uint32_t
{
    Available = 1,
    Reserved = 2,
    AcpiReclaimable = 3,
    Nvs = 4,
    BadRam = 5,
};

struct [[gnu::packed]] MmapEntry
{
    uint64_t addr;
    uint64_t len;
    MemoryType type;
    uint32_t zero;
};

struct [[gnu::packed]] Tag
{
    TagType type;
    uint32_t size;
};

struct [[gnu::packed]] StringTag : public Tag
{
    char string[0];
};

struct [[gnu::packed]] ModuleTag : public Tag
{
    uint32_t modStart;
    uint32_t modEnd;
    char cmdline[0];
};

struct [[gnu::packed]] BasicMeminfoTag : public Tag
{
    uint32_t memLower;
    uint32_t memUpper;
};

struct [[gnu::packed]] BootdevTag : public Tag
{
    uint32_t biosdev;
    uint32_t slice;
    uint32_t part;
};

struct [[gnu::packed]] MmapTag : public Tag
{
    uint32_t entrySize;
    uint32_t entryVersion;
    MmapEntry entries[0];
};

struct [[gnu::packed]] VbeInfoBlock
{
    uint8_t externalSpecification[512];
};

struct [[gnu::packed]] VbeModeInfoBlock
{
    uint8_t externalSpecification[256];
};

struct [[gnu::packed]] VbeTag : public Tag
{
    uint16_t vbeMode;
    uint16_t vbeInterfaceSeg;
    uint16_t vbeInterfaceOff;
    uint16_t vbeInterfaceLen;

    VbeInfoBlock vbeControlInfo;
    VbeModeInfoBlock vbeModeInfo;
};

enum class FramebufferType : uint32_t
{
    Indexed = 0,
    Rgb = 1,
    EgaText = 2,
};

struct [[gnu::packed]] FramebufferCommonTag : public Tag
{
    uint64_t framebufferAddr;
    uint32_t framebufferPitch;
    uint32_t framebufferWidth;
    uint32_t framebufferHeight;
    uint8_t framebufferBpp;
    uint8_t framebufferType;
    uint16_t reserved;
};

struct [[gnu::packed]] Color
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

struct [[gnu::packed]] FramebufferTag : FramebufferCommonTag
{
    union {
        struct
        {
            uint16_t framebufferPaletteNumColors;
            Color framebufferPalette[0];
        };
        struct
        {
            uint8_t framebufferRedFieldPosition;
            uint8_t framebufferRedMaskSize;
            uint8_t framebufferGreenFieldPosition;
            uint8_t framebufferGreenMaskSize;
            uint8_t framebufferBlueFieldPosition;
            uint8_t framebufferBlueMaskSize;
        };
    };
};

struct [[gnu::packed]] ElfSectionsTag : public Tag
{
    uint32_t num;
    uint32_t entsize;
    uint32_t shndx;
    char sections[0];
};

struct [[gnu::packed]] ApmTag : public Tag
{
    uint16_t version;
    uint16_t cseg;
    uint32_t offset;
    uint16_t cseg16;
    uint16_t dseg;
    uint16_t flags;
    uint16_t csegLen;
    uint16_t cseg16Len;
    uint16_t dsegLen;
};

struct [[gnu::packed]] Efi32Tag : public Tag
{
    uint32_t pointer;
};

struct [[gnu::packed]] Efi64Tag : public Tag
{
    uint64_t pointer;
};

struct [[gnu::packed]] SmbiosTag : public Tag
{
    uint8_t major;
    uint8_t minor;
    uint8_t reserved[6];
    uint8_t tables[0];
};

struct [[gnu::packed]] OldAcpiTag : public Tag
{
    uint8_t rsdp[0];
};

struct [[gnu::packed]] NewAcpiTag : public Tag
{
    uint8_t rsdp[0];
};

struct [[gnu::packed]] NetworkTag : public Tag
{
    uint8_t dhcpack[0];
};

struct [[gnu::packed]] EfiMmapTag : public Tag
{
    uint32_t descrSize;
    uint32_t descrVers;
    uint8_t efiMmap[0];
};

struct [[gnu::packed]] Efi32IhTag : public Tag
{
    uint32_t pointer;
};

struct [[gnu::packed]] Efi64IhTag : public Tag
{
    uint64_t pointer;
};

struct [[gnu::packed]] LoadBaseAddrTag : public Tag
{
    uint32_t loadBaseAddr;
};

struct [[gnu::packed]] Info
{
    uint32_t totalSize;
    uint32_t reserved;
};

struct TagEnd {};

class TagIterator
{
public:
    TagIterator(const Tag* current = nullptr) :
        m_current(current)
    {
    }

    const Tag& operator*() const
    {
        return *m_current;
    }

    TagIterator& operator++()
    {
        auto addr = reinterpret_cast<uintptr_t>(m_current);
        addr += m_current->size;

        // multiboot tags are 8-byte aligned
        addr = stl::align(8, addr);
        m_current = reinterpret_cast<const Tag*>(addr);

        return *this;
    }

    bool operator==(const TagEnd&) const
    {
        return m_current->type == TagType::End && m_current->size == 8;
    }

    bool operator!=(const TagEnd& endTag) const
    {
        return !((*this) == endTag);
    }

private:
    const Tag* m_current;
};

inline TagIterator begin(const Info* info)
{
    auto addr = reinterpret_cast<uintptr_t>(info);
    addr += sizeof(*info);

    return TagIterator(reinterpret_cast<const Tag*>(addr));
}

inline TagEnd end(const Info*)
{
    return TagEnd{};
}

}