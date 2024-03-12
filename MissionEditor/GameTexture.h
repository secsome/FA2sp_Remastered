#pragma once

// Helper to make texture

#include <d3d11.h>

namespace GameTexture
{
    HRESULT MakeTexture(ID3D11Device* device, ID3D11Texture2D** texture, UINT width, UINT height, const void* mem, UINT pitch);
    // Assume width == pitch
    HRESULT MakeTexture(ID3D11Device* device, ID3D11Texture2D** texture, UINT width, UINT height, const void* mem);
}
