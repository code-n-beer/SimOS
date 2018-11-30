#pragma once

#include <stdint.h>

#define PACKED __attribute__((packed))

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

struct PACKED MmapEntry
{
    uint64_t addr;
    uint64_t len;
    MemoryType type;
    uint32_t zero;
};

struct PACKED Tag
{
    TagType type;
    uint32_t size;
};

struct PACKED StringTag : public Tag
{
    char string[0];
};

struct PACKED ModuleTag : public Tag
{
    uint32_t modStart;
    uint32_t modEnd;
    char cmdline[0];
};

struct PACKED BasicMeminfoTag : public Tag
{
    uint32_t memLower;
    uint32_t memUpper;
};

struct PACKED BootdevTag : public Tag
{
    uint32_t biosdev;
    uint32_t slice;
    uint32_t part;
};

struct PACKED MmapTag : public Tag
{
    uint32_t entrySize;
    uint32_t entryVersion;
    MmapEntry entries[0];
};

struct PACKED VbeInfoBlock
{
    uint8_t externalSpecification[512];
};

struct PACKED VbeModeInfoBlock
{
    uint8_t externalSpecification[256];
};

struct PACKED VbeTag : public Tag
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

struct PACKED FramebufferCommonTag : public Tag
{
    uint64_t framebufferAddr;
    uint32_t framebufferPitch;
    uint32_t framebufferWidth;
    uint32_t framebufferHeight;
    uint8_t framebufferBpp;
    uint8_t framebufferType;
    uint16_t reserved;
};

struct PACKED Color
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

struct PACKED FramebufferTag : FramebufferCommonTag
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

struct PACKED ElfSectionsTag : public Tag
{
    uint32_t num;
    uint32_t entsize;
    uint32_t shndx;
    char sections[0];
};

struct PACKED ApmTag : public Tag
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

struct PACKED Efi32Tag : public Tag
{
    uint32_t pointer;
};

struct PACKED Efi64Tag : public Tag
{
    uint64_t pointer;
};

struct PACKED SmbiosTag : public Tag
{
    uint8_t major;
    uint8_t minor;
    uint8_t reserved[6];
    uint8_t tables[0];
};

struct PACKED OldAcpiTag : public Tag
{
    uint8_t rsdp[0];
};

struct PACKED NewAcpiTag : public Tag
{
    uint8_t rsdp[0];
};

struct PACKED NetworkTag : public Tag
{
    uint8_t dhcpack[0];
};

struct PACKED EfiMmapTag : public Tag
{
    uint32_t descrSize;
    uint32_t descrVers;
    uint8_t efiMmap[0];
};

struct PACKED Efi32IhTag : public Tag
{
    uint32_t pointer;
};

struct PACKED Efi64IhTag : public Tag
{
    uint64_t pointer;
};

struct PACKED LoadBaseAddrTag : public Tag
{
    uint32_t loadBaseAddr;
};

struct PACKED MultibootBasicInfo
{
    uint32_t totalSize;
    uint32_t reserved;
};

struct MultibootTagEnd {};

class MultibootTagIterator
{
public:
    MultibootTagIterator(const Tag* current = nullptr) :
        m_current(current)
    {
    }

    const Tag& operator*() const
    {
        return *m_current;
    }

    MultibootTagIterator& operator++()
    {
        auto addr = reinterpret_cast<uintptr_t>(m_current);
        addr += m_current->size;

        // multiboot tags are 8-byte aligned
        addr = ((addr - 1) & ~0x7) + 0x8;
        m_current = reinterpret_cast<const Tag*>(addr);

        return *this;
    }

    bool operator==(const MultibootTagEnd&) const
    {
        return m_current->type == TagType::End && m_current->size == 8;
    }

    bool operator!=(const MultibootTagEnd& endTag) const
    {
        return !((*this) == endTag);
    }

private:
    const Tag* m_current;
};

inline MultibootTagIterator begin(const MultibootBasicInfo* basicInfo)
{
    auto addr = reinterpret_cast<uintptr_t>(basicInfo);
    addr += sizeof(*basicInfo);

    return MultibootTagIterator(reinterpret_cast<const Tag*>(addr));
}

inline MultibootTagEnd end(const MultibootBasicInfo*)
{
    return MultibootTagEnd{};
}

