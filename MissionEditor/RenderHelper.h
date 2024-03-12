#pragma once

// Helper to make texture

#include <d3d11.h>
#include <DirectXMath.h>

#include "Convert.h"
#include "VxlFile.h"
#include "VplFile.h"

namespace RenderHelper
{
    HRESULT MakeConvertTexture(ID3D11Device* device, ID3D11Texture2D** texture, UINT width, UINT height, const void* mem, UINT pitch);
    // Assume width == pitch
    HRESULT MakeConvertTexture(ID3D11Device* device, ID3D11Texture2D** texture, UINT width, UINT height, const void* mem);

    // Create a Direct2D usable bitmap from a Direct3D texture 2D
    HRESULT MakeBitmap(ID2D1RenderTarget* RT, ID3D11Texture2D* texture, const ConvertClass& convert, ID2D1Bitmap** bitmap);

    // This function is referenced from WorldAlteringEditor/Rendering/VoxelRenderer
    auto MakeVoxelTexture(ID3D11Device* device, ID3D11Texture2D** texture, float angle, uint8_t ramp,
        const VxlFile& vxl, const ConvertClass& convert, const VplFile* vpl, bool remap = false) -> std::pair<int32_t, int32_t>;
}
