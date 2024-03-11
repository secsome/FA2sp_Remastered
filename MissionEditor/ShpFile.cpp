#include "StdAfx.h"

#include "ShpFile.h"

#include "CCFile.h"

ShpFile::ShpFile(const char* filename)
{
    CCFileClass file{ filename };
    Data.reset(reinterpret_cast<ShpStruct*>(new char[file.Size()]));
    if (file.Read(Data.get(), file.Size()) != file.Size())
        throw std::runtime_error("Failed to read SHP file");
    file.Close();
}

ShpFile::ShpFile(const void* buffer, size_t size, bool copy)
{
    if (copy)
    {
        Data.reset(reinterpret_cast<ShpStruct*>(new char[size]));
        std::memcpy(Data.get(), buffer, size);
    }
    else
        Data.reset(reinterpret_cast<ShpStruct*>(const_cast<void*>(buffer)));
}

bool ShpFile::IsTextureLoaded() const
{
    return !FrameTextures.empty();
}

bool ShpFile::IsFrameTextureLoaded(size_t index) const
{
    if (index >= FrameTextures.size())
        return false;

    return FrameTextures[index] != nullptr;
}

bool ShpFile::LoadTexture(ID3D11Device* device, size_t index)
{
    if (device == nullptr)
        return false;

    if (!IsTextureLoaded())
        FrameTextures.resize(Data->Count);

    if (index >= FrameTextures.size())
        return false;

    if (FrameTextures[index] != nullptr)
        return true;

    const auto& frame = Data->Frames[index];
    const auto data = Data->GetFrameData(index);

    if (frame.Width <= 0 || frame.Height <= 0)
        return false;

    CComPtr<ID3D11Texture2D> texture;
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = frame.Width;
    desc.Height = frame.Height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_A8_UNORM; // only 8-bit index is stored in the texture
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    std::vector<uint8_t> pixels;
    pixels.resize(frame.Width * frame.Height);

    auto src = data;
    auto dst = pixels.data();

    if (frame.IsCompressed())
    {
        for (size_t y = 0; y < frame.Height; ++y)
        {
            const auto line_length = src[0] | (src[1] << 8);
            const auto end = src + line_length;
            const auto beg = dst;
            while (src < end)
            {
                if (const auto color = *src++)
                    *dst++ = color;
                else
                    dst += color;
            }
            dst = beg + frame.Width;
        }
    }
    else
    {
        for (size_t y = 0; y < frame.Height; ++y)
        {
            std::memcpy(dst, src, frame.Width);
            src += frame.Width;
            dst += frame.Width;
        }
    }

    D3D11_SUBRESOURCE_DATA init{};
    init.pSysMem = pixels.data();
    init.SysMemPitch = frame.Width;
    
    if (FAILED(device->CreateTexture2D(&desc, &init, &texture)))
        return false;
    
    FrameTextures[index] = texture;
    
    return true;
}

void ShpFile::ClearTextures()
{
    FrameTextures.clear();
}
