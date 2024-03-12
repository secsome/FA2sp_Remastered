#pragma once

#include <map>
#include <memory>
#include <atlbase.h>
#include <cstdint>

#include <d3d11.h>

class TmpFile
{
private:
    struct TmpCellHeader
    {
        int32_t X;
        int32_t Y;
        uint32_t ExtraDataOffset;
        uint32_t ZDataOffset;
        uint32_t ExtraZDataOffset;
        int32_t ExtraX;
        int32_t ExtraY;
        uint32_t ExtraWidth;
        uint32_t ExtraHeight;
        uint8_t Type;
        uint8_t Alignment[3];
        uint8_t Height;
        uint8_t LandType;
        uint8_t RampType;
        uint8_t LowRadarColor[3];
        uint8_t HighRadarColor[3];
        uint8_t Padding[2];

        bool HasExtraData() const
        {
            return (Type & 1) != 0;
        }

        bool HasZData() const
        {
            return (Type & 2) != 0;
        }

        bool HasDamagedData() const
        {
            return (Type & 4) != 0;
        }
    };
    static_assert(sizeof(TmpCellHeader) == 52);
    
    struct TmpStruct
    {
        uint32_t Width;
        uint32_t Height;
        uint32_t ImageWidth;
        uint32_t ImageHeight;
        uint32_t Offsets[1];

        TmpCellHeader* GetCellHeader(size_t index)
        {
            return Offsets[index] ? 
                reinterpret_cast<TmpCellHeader*>(reinterpret_cast<uint8_t*>(this) + Offsets[index]) : 
                nullptr;
        }
    };
    static_assert(sizeof(TmpStruct) == 20);

public:
    explicit TmpFile(const char* filename);
    explicit TmpFile(const void* buffer, size_t size, bool copy);

    bool IsTextureLoaded() const;
    bool LoadAllTexture(ID3D11Device* device);
    void ClearTextures();

    ID3D11Texture2D* GetTexture(size_t tile) const
    {
        auto const it = CellTextures.find(tile);
        return it != CellTextures.end() ? it->second : nullptr;
    }

    ID3D11Texture2D* GetTexture(size_t x, size_t y) const
    {
        return GetTexture(x + y * Data->Width);
    }

    ID3D11Texture2D* GetZTexture(size_t tile) const
    {
        auto const it = CellZTextures.find(tile);
        return it != CellZTextures.end() ? it->second : nullptr;
    }

    ID3D11Texture2D* GetZTexture(size_t x, size_t y) const
    {
        return GetZTexture(x + y * Data->Width);
    }

    ID3D11Texture2D* GetExtraTexture(size_t tile) const
    {
        auto const it = CellExtraTextures.find(tile);
        return it != CellExtraTextures.end() ? it->second : nullptr;
    }

    ID3D11Texture2D* GetExtraTexture(size_t x, size_t y) const
    {
        return GetExtraTexture(x + y * Data->Width);
    }

    ID3D11Texture2D* GetExtraZTexture(size_t tile) const
    {
        auto const it = CellExtraZTextures.find(tile);
        return it != CellExtraZTextures.end() ? it->second : nullptr;
    }

    ID3D11Texture2D* GetExtraZTexture(size_t x, size_t y) const
    {
        return GetExtraZTexture(x + y * Data->Width);
    }

private:
    std::unique_ptr<TmpStruct> Data;
    size_t TileCount;
    std::map<size_t, CComPtr<ID3D11Texture2D>> CellTextures;
    std::map<size_t, CComPtr<ID3D11Texture2D>> CellZTextures;
    std::map<size_t, CComPtr<ID3D11Texture2D>> CellExtraTextures;
    std::map<size_t, CComPtr<ID3D11Texture2D>> CellExtraZTextures;
};