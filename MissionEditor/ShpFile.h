#pragma once

#include <vector>
#include <cstdint>

#include <d3d11.h>

class ShpFile
{
private:
    struct ShpFrame
    {
        std::int16_t X;
        std::int16_t Y;
        std::int16_t Width;
        std::int16_t Height;
        std::uint8_t Flags;
        std::uint8_t Alignment[3];
        std::uint32_t Color;
        std::uint32_t Unused;
        std::uint32_t Offset;

        bool IsCompressed() const
        {
            return (Flags & 2) != 0;
        }
    };
    static_assert(sizeof(ShpFrame) == 24);

    struct ShpStruct
    {
        uint16_t Type;
        uint16_t Width;
        uint16_t Height;
        uint16_t Count;
        ShpFrame Frames[1];

        uint8_t* GetFrameData(size_t index)
        {
            return reinterpret_cast<uint8_t*>(this) + Frames[index].Offset;
        }
    };
    static_assert(sizeof(ShpStruct) == 8 + sizeof(ShpFrame));

public:
    explicit ShpFile(const char* filename);
    explicit ShpFile(const void* buffer, size_t size, bool copy);

    bool IsTextureLoaded() const;
    bool IsFrameTextureLoaded(size_t index) const;
    bool LoadTexture(ID3D11Device* device, size_t index);
    void ClearTextures();

    size_t FrameCount() const
    {
        return Data->Count;
    }

    ID3D11Texture2D* GetTexture(size_t index) const
    {
        return FrameTextures[index];
    }

private:
    std::unique_ptr<ShpStruct> Data;
    std::vector<CComPtr<ID3D11Texture2D>> FrameTextures;
};