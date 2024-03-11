#pragma once

#include <array>
#include <cstdint>

struct RGBStruct
{
    uint8_t R;
    uint8_t G;
    uint8_t B;
};
static_assert(sizeof(RGBStruct) == 3);

struct HSVStruct
{
    uint8_t H;
    uint8_t S;
    uint8_t V;
};
static_assert(sizeof(HSVStruct) == 3);

class ConvertClass
{
public:
    explicit ConvertClass(const char* filename);
    explicit ConvertClass(const void* buffer, size_t size);

    static constexpr size_t RemapStart = 16;
    static constexpr size_t RemapEnd = 32;

    void Remap(RGBStruct color);

private:
    std::array<RGBStruct, 256> Palette;
    std::array<RGBStruct, 256> OriginalPalette;
};