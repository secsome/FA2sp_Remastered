#pragma once

#include <vector>
#include <memory>
#include <atlbase.h>
#include <cstdint>

#include <d3d11.h>

class VplFile
{
    struct VplSection
    {
        uint8_t Table[256];
    };
    static_assert(sizeof(VplSection) == 256);

    struct VplStruct
    {
        int32_t RemapStart;
        int32_t RemapEnd;
        int32_t SectionCount;
        uint32_t Unused;
        uint8_t EmbeddedPalette[256][3];
        VplSection Section[1];
    };
    static_assert(sizeof(VplStruct) == 784 + sizeof(VplSection));
public:
    explicit VplFile(const char* filename);
    explicit VplFile(const void* buffer, size_t size, bool copy);

    uint8_t GetPaletteIndex(uint8_t page, uint8_t color) const
    {
        return Data->Section[page].Table[color];
    }

private:
    std::unique_ptr<VplStruct> Data;

};