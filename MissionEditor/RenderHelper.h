#pragma once

// Helper to make texture

#include <d3d11.h>

#include "Convert.h"

namespace RenderHelper
{
    HRESULT MakeConvertTexture(ID3D11Device* device, ID3D11Texture2D** texture, UINT width, UINT height, const void* mem, UINT pitch);
    // Assume width == pitch
    HRESULT MakeConvertTexture(ID3D11Device* device, ID3D11Texture2D** texture, UINT width, UINT height, const void* mem);

    HRESULT MakeBitmap(ID2D1RenderTarget* RT, ID3D11Texture2D* texture, const ConvertClass& convert, ID2D1Bitmap** bitmap);
}
