#include "StdAfx.h"

#include "RenderHelper.h"

#include <vector>

#include <DirectXMath.h>
#include <DirectXColors.h>


using namespace DirectX;

namespace RenderHelper
{
    HRESULT MakeConvertTexture(ID3D11Device* device, ID3D11Texture2D** texture, UINT width, UINT height, const void* mem, UINT pitch)
    {
        D3D11_TEXTURE2D_DESC desc{};
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8_UINT; // only 8-bit index is stored in the texture
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        desc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA init{};
        init.pSysMem = mem;
        init.SysMemPitch = pitch;

        return device->CreateTexture2D(&desc, &init, texture);
    }

    HRESULT MakeConvertTexture(ID3D11Device* device, ID3D11Texture2D** texture, UINT width, UINT height, const void* mem)
    {
        return MakeConvertTexture(device, texture, width, height, mem, width);
    }

    HRESULT MakeBitmap(ID2D1RenderTarget* RT, ID3D11Texture2D* texture, const ConvertClass& convert, ID2D1Bitmap** bitmap)
    {
        HRESULT hr = S_OK;

        D3D11_TEXTURE2D_DESC desc{};
        texture->GetDesc(&desc);

        if (desc.Width <= 0 || desc.Height <= 0)
            return E_FAIL;

        CComPtr<ID3D11Device> device;
        texture->GetDevice(&device);

        CComPtr<ID3D11DeviceContext> context;
        device->GetImmediateContext(&context);

        CComPtr<ID3D11Texture2D> staging;
        desc.Usage = D3D11_USAGE_STAGING;
        desc.BindFlags = 0;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        desc.MiscFlags = 0;
        hr = device->CreateTexture2D(&desc, nullptr, &staging);
        if (FAILED(hr))
            return hr;

        context->CopyResource(staging, texture);

        D3D11_MAPPED_SUBRESOURCE mapped;
        hr = context->Map(staging, 0, D3D11_MAP_READ, 0, &mapped);
        if (FAILED(hr))
            return hr;

        std::vector<uint8_t> pixels;
        pixels.resize(desc.Width * desc.Height * 4);
        auto dst = pixels.data();
        auto src = static_cast<const uint8_t*>(mapped.pData);
        for (size_t y = 0; y < desc.Height; ++y)
        {
            auto line_ptr = src;
            for (size_t x = 0; x < desc.Width; ++x)
            {
                const auto index = *line_ptr++;
                if (index)
                {
                    const auto& [r, g, b] = convert[index];
                    dst[0] = r; dst[1] = g; dst[2] = b; dst[3] = 1;
                }
                else
                {
                    dst[0] = dst[1] = dst[2] = 0;
                    dst[3] = 1;
                }
                dst += 4;
            }
            src += mapped.RowPitch;
        }

        context->Unmap(staging, 0);

        D2D1_BITMAP_PROPERTIES props = {};
        props.dpiX = 0.0f;
        props.dpiY = 0.0f;
        props.pixelFormat.format = DXGI_FORMAT_R8G8B8A8_UNORM;
        props.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;

        D2D1_SIZE_U size;
        size.width = desc.Width;
        size.height = desc.Height;

        hr = RT->CreateBitmap(size, pixels.data(), size.width * 4, &props, bitmap);
        if (FAILED(hr))
            return hr;

        return S_OK;
    }


}
