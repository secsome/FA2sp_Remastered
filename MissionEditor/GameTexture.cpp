#include "StdAfx.h"

#include "GameTexture.h"

namespace GameTexture
{
    HRESULT MakeTexture(ID3D11Device* device, ID3D11Texture2D** texture, UINT width, UINT height, const void* mem, UINT pitch)
    {
        D3D11_TEXTURE2D_DESC desc{};
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8_UINT; // only 8-bit index is stored in the texture
        desc.SampleDesc.Count = 1;
#ifdef _DEBUG
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        desc.Usage = D3D11_USAGE_DEFAULT;
#else
        desc.CPUAccessFlags = 0;
        desc.Usage = D3D11_USAGE_IMMUTABLE;
#endif
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA init{};
        init.pSysMem = mem;
        init.SysMemPitch = pitch;

        return device->CreateTexture2D(&desc, &init, texture);
    }

    HRESULT MakeTexture(ID3D11Device* device, ID3D11Texture2D** texture, UINT width, UINT height, const void* mem)
    {
        return MakeTexture(device, texture, width, height, mem, width);
    }
}
