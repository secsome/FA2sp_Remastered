#include "StdAfx.h"

#include "TmpFile.h"

#include "CCFile.h"
#include "GameTexture.h"

#include <vector>

TmpFile::TmpFile(const char* filename)
{
    CCFileClass file{ filename };
    Data.reset(reinterpret_cast<TmpStruct*>(new char[file.Size()]));
    if (file.Read(Data.get(), file.Size()) != file.Size())
        throw std::runtime_error("Failed to read TMP file");
    file.Close();

    if (Data->Width >= 256 || Data->Height >= 256)
        throw std::runtime_error("Invalid TMP header width or height");
    
    TileCount = Data->Width * Data->Height;
}

TmpFile::TmpFile(const void* buffer, size_t size, bool copy)
{
    if (copy)
    {
        Data.reset(reinterpret_cast<TmpStruct*>(new char[size]));
        std::memcpy(Data.get(), buffer, size);
    }
    else
        Data.reset(reinterpret_cast<TmpStruct*>(const_cast<void*>(buffer)));

    if (Data->Width >= 256 || Data->Height >= 256)
        throw std::runtime_error("Invalid TMP header width or height");

    TileCount = Data->Width * Data->Height;
}

bool TmpFile::IsTextureLoaded() const
{
    return !CellTextures.empty();
}

bool TmpFile::LoadAllTexture(ID3D11Device* device)
{
    if (device == nullptr)
        return false;

    if (IsTextureLoaded())
        return true;

    for (size_t k = 0; k < Data->Width * Data->Height; ++k)
    {
        const auto header = Data->GetCellHeader(k);
        if (header == nullptr)
            continue;

        // Pixel Texture
        {
            std::vector<uint8_t> pixels;
            pixels.resize(Data->ImageWidth * Data->ImageHeight);
            {
                auto src = reinterpret_cast<uint8_t*>(header) + sizeof(TmpCellHeader);

                size_t count = 0;
                for (int32_t y = 0; y < Data->ImageHeight - 1; ++y)
                {
                    const size_t line_length = Data->ImageWidth - std::abs(static_cast<int32_t>(Data->ImageHeight / 2) - y - 1) * 4;
                    const size_t x_start = (Data->ImageWidth - line_length) / 2;
                    for (size_t x = x_start; x < x_start + line_length; ++x)
                        pixels[x + y * Data->ImageWidth] = *src++;
                }
            }

            CComPtr<ID3D11Texture2D> texture;
            if (FAILED(GameTexture::MakeTexture(device, &texture, Data->ImageWidth, Data->ImageHeight, pixels.data())))
                return false;

            CellTextures[k] = texture;
        }

        // Pixel ZTexture
        if (header->HasZData())
        {
            std::vector<uint8_t> pixels;
            pixels.resize(Data->ImageWidth * Data->ImageHeight);
            {
                auto src = reinterpret_cast<uint8_t*>(header) + sizeof(TmpCellHeader) + 
                    Data->ImageWidth * Data->ImageHeight / 2;

                size_t count = 0;
                for (int32_t y = 0; y < Data->ImageHeight - 1; ++y)
                {
                    const size_t line_length = Data->ImageWidth - std::abs(static_cast<int32_t>(Data->ImageHeight / 2) - y - 1) * 4;
                    const size_t x_start = (Data->ImageWidth - line_length) / 2;
                    for (size_t x = x_start; x < x_start + line_length; ++x)
                        pixels[x + y * Data->ImageWidth] = *src++;
                }
            }

            CComPtr<ID3D11Texture2D> texture;
            if (FAILED(GameTexture::MakeTexture(device, &texture, Data->ImageWidth, Data->ImageHeight, pixels.data())))
                return false;

            CellZTextures[k] = texture;
        }

        if (!header->HasExtraData())
            continue;
        
        // Extra Pixel Texture
        {
            auto src = reinterpret_cast<uint8_t*>(header) + sizeof(TmpCellHeader) +
                Data->ImageWidth * Data->ImageHeight;

            CComPtr<ID3D11Texture2D> texture;
            if (FAILED(GameTexture::MakeTexture(device, &texture, header->ExtraWidth, header->ExtraHeight, src)))
                return false;

            CellExtraTextures[k] = texture;
        }

        // Extra Pixel ZTexture
        if (header->HasZData())
        {
            auto src = reinterpret_cast<uint8_t*>(header) + sizeof(TmpCellHeader) +
                Data->ImageWidth * Data->ImageHeight +
                header->ExtraWidth * header->ExtraHeight;

            CComPtr<ID3D11Texture2D> texture;
            if (FAILED(GameTexture::MakeTexture(device, &texture, header->ExtraWidth, header->ExtraHeight, src)))
                return false;

            CellExtraZTextures[k] = texture;
        }
    }
    
    return true;
}

void TmpFile::ClearTextures()
{
    CellTextures.clear();
    CellZTextures.clear();
    CellExtraTextures.clear();
    CellExtraZTextures.clear();
}
