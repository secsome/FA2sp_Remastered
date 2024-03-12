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

    struct VertexPositionColor
    {
        XMVECTOR Position;
        RGBStruct Color;
    };
    inline void RenderVoxel(XMVECTOR position, RGBStruct color, size_t vertexIndexCount,
        std::vector<int>& vertexIndices, std::array<VertexPositionColor, 8>& verticesArray)
    {
        std::array<XMVECTOR, 8> vertexCoordinates
        {
            XMVectorSet(-1.0f, 1.0f, -1.0f, 0.0f),
            XMVectorSet(1.0f, 1.0f, -1.0f, 0.0f),
            XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f),
            XMVectorSet(-1.0f, 1.0f, 1.0f, 0.0f),
            XMVectorSet(-1.0f, -1.0f, -1.0f, 0.0f),
            XMVectorSet(1.0f, -1.0f, -1.0f, 0.0f),
            XMVectorSet(1.0f, -1.0f, 1.0f, 0.0f),
            XMVectorSet(-1.0f, -1.0f, 1.0f, 0.0f)
        };

        constexpr float radius = 0.5f;
        for (auto& coordinate : vertexCoordinates)
        {
            coordinate *= radius;
            coordinate += position;
        }

        for (size_t i = 0; i < verticesArray.size(); ++i)
        {
            verticesArray[i].Position = vertexCoordinates[i];
            verticesArray[i].Color = color;
        }

        constexpr std::array<int, 3> VertexIndexTriangles[12]
        {
            { 0, 1, 2 }, { 2, 3, 0 }, // up
            { 7, 6, 5 }, { 5, 4, 7 }, // down
            { 4, 5, 1 }, { 1, 0, 4 }, // forward
            { 3, 2, 6 }, { 6, 7, 3 }, // backward
            { 1, 5, 6 }, { 6, 2, 1 }, // right
            { 4, 0, 3 }, { 3, 7, 4 }, // left
        };
        for (const auto& [v1, v2, v3] : VertexIndexTriangles)
        {
            vertexIndices.push_back(v1 + vertexIndexCount);
            vertexIndices.push_back(v2 + vertexIndexCount);
            vertexIndices.push_back(v3 + vertexIndexCount);
        }
    }

    inline auto PreCalculateLighting(const NormalTableType& normal, int mode, float rotation) -> std::array<uint8_t, 256>
    {
        static const XMVECTOR YRLight = XMVector3Transform(-g_XMIdentityR0, XMMatrixRotationZ(XMConvertToRadians(45.0f)));

        XMVECTOR light = XMVector3Transform(YRLight, XMMatrixRotationZ(rotation - XMConvertToRadians(45.0f)));
        const uint8_t centerPage = 7;
        int maxPage = mode * 7 - 1;
        
        std::array<uint8_t, 256> normalIndexToVplPage;
        for (size_t i = 0; i < normal.size(); ++i)
        {
            float dot = (XMVector3Dot(normal[i], light).m128_f32[0] + 1) / 2;
            uint8_t page = dot <= 0.5 ? 
                static_cast<uint8_t>(dot * 2 * centerPage) :
                static_cast<uint8_t>(2 * (maxPage - centerPage) * (dot - 0.5f) + centerPage);
            normalIndexToVplPage[i] = page;
        }

        normalIndexToVplPage[253] = 16;
        normalIndexToVplPage[254] = 16;
        normalIndexToVplPage[255] = 16;

        return normalIndexToVplPage;
    }

    RECT inline GetSectionBounds(float x, float y, float z, XMMATRIX world, const XMMATRIX& section)
    {
        world = section * world;

        XMVECTOR floorTopLeft = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
        XMVECTOR floorTopRight = XMVectorSet(x, 0.0f, 0.0f, 0.0f);
        XMVECTOR floorBottomRight = XMVectorSet(x, y, 0.0f, 0.0f);
        XMVECTOR floorBottomLeft = XMVectorSet(0.0f, y, 0.0f, 0.0f);
        XMVECTOR ceilTopLeft = XMVectorSet(0.0f, 0.0f, z, 0.0f);
        XMVECTOR ceilTopRight = XMVectorSet(x, 0.0f, z, 0.0f);
        XMVECTOR ceilBottomRight = XMVectorSet(x, y, z, 0.0f);
        XMVECTOR ceilBottomLeft = XMVectorSet(0.0f, y, z, 0.0f);

        floorTopLeft = XMVector3Transform(floorTopLeft, world);
        floorTopRight = XMVector3Transform(floorTopRight, world);
        floorBottomRight = XMVector3Transform(floorBottomRight, world);
        floorBottomLeft = XMVector3Transform(floorBottomLeft, world);
        ceilTopLeft = XMVector3Transform(ceilTopLeft, world);
        ceilTopRight = XMVector3Transform(ceilTopRight, world);
        ceilBottomRight = XMVector3Transform(ceilBottomRight, world);
        ceilBottomLeft = XMVector3Transform(ceilBottomLeft, world);

        int FminX = (int)std::floor(std::min({ floorTopLeft.m128_f32[0], floorTopRight.m128_f32[0], floorBottomLeft.m128_f32[0], floorBottomRight.m128_f32[0] }));
        int FmaxX = (int)std::ceil(std::max({ floorTopLeft.m128_f32[0], floorTopRight.m128_f32[0], floorBottomLeft.m128_f32[0], floorBottomRight.m128_f32[0] }));
        int FminY = (int)std::floor(std::min({ floorTopLeft.m128_f32[1], floorTopRight.m128_f32[1], floorBottomLeft.m128_f32[1], floorBottomRight.m128_f32[1] }));
        int FmaxY = (int)std::ceil(std::max({ floorTopLeft.m128_f32[1], floorTopRight.m128_f32[1], floorBottomLeft.m128_f32[1], floorBottomRight.m128_f32[1] }));
        int TminX = (int)std::floor(std::min({ ceilTopLeft.m128_f32[0], ceilTopRight.m128_f32[0], ceilBottomLeft.m128_f32[0], ceilBottomRight.m128_f32[0] }));
        int TmaxX = (int)std::ceil(std::max({ ceilTopLeft.m128_f32[0], ceilTopRight.m128_f32[0], ceilBottomLeft.m128_f32[0], ceilBottomRight.m128_f32[0] }));
        int TminY = (int)std::floor(std::min({ ceilTopLeft.m128_f32[1], ceilTopRight.m128_f32[1], ceilBottomLeft.m128_f32[1], ceilBottomRight.m128_f32[1] }));
        int TmaxY = (int)std::ceil(std::max({ ceilTopLeft.m128_f32[1], ceilTopRight.m128_f32[1], ceilBottomLeft.m128_f32[1], ceilBottomRight.m128_f32[1] }));

        RECT ret;

        ret.left = std::min(FminX, TminX);
        ret.right = std::max(FmaxX, TmaxX);
        ret.top = std::min(FminY, TminY);
        ret.bottom = std::max(FmaxY, TmaxY);

        return ret;
    }

    auto MakeVoxelTexture(ID3D11Device* device, ID3D11Texture2D** texture, float angle, uint8_t ramp, const VxlFile& vxl, const ConvertClass& convert, const VplFile* vpl, bool remap) -> std::pair<int32_t, int32_t>
    {
        constexpr std::array<int, 20> SlopeAxisZAngles
        {
            135, -135, -45, 45,
            180, -90, 0, 90,
            180, -90, 0, 90,
            180, -90, 0, 90,
            180, -90, 0, 90
        };
        constexpr float SlopeAngle = XMConvertToRadians(27.5f);
        constexpr float ModelScale = 0.025f;
        static const XMMATRIX Scale = XMMatrixScaling(ModelScale, ModelScale, ModelScale);

        texture = nullptr;
        if (device == nullptr)
            return { 0,0 };

        XMMATRIX tilt = XMMatrixIdentity();
        if (ramp > 0 && ramp < 17)
        {
            XMMATRIX rotateTiltAxis = XMMatrixRotationZ(XMConvertToRadians(SlopeAxisZAngles[ramp - 1]));
            rotateTiltAxis *= XMMatrixRotationX(XMConvertToRadians(-60.0f));
            XMVECTOR tiltAxis = XMVector3Transform(g_XMIdentityR0, rotateTiltAxis);
            tilt = XMMatrixRotationAxis(tiltAxis, SlopeAngle);
        }

        XMMATRIX rotateToWorld = XMMatrixRotationZ(XMConvertToRadians(45.0f) - angle);
        rotateToWorld *= XMMatrixRotationX(XMConvertToRadians(-60.0f));
        XMMATRIX world = rotateToWorld * tilt * Scale;

        std::vector<VertexPositionColor> vertexData;
        std::vector<int32_t> vertexIndices;

        RECT imageBounds = { 0 };
        std::array<VertexPositionColor, 8> verticesArray;
        for (size_t i = 0; i < vxl.GetLayerCount(); ++i)
        {
            const auto& body = vxl.LayerBodies[i];
            const auto& info = vxl.LayerInfos[i];

            const auto normalIndexToVplPage = PreCalculateLighting(
                vxl.GetNormalTable(i), info.NormalType, angle);
            
            auto sectionHvaTransform = vxl.GetHvaMatrix(vxl.LayerHeaders[i].LayerIndex, 0);
            sectionHvaTransform.r[3] *= info.Scale;

            const auto sectionTranslation = XMMatrixTranslation(
                info.MinBounds[0], info.MinBounds[1], info.MinBounds[2]);

            const auto sectionScale = XMMatrixScaling(
                (info.MaxBounds[0] - info.MinBounds[0]) * 1.0f / info.SizeX,
                (info.MaxBounds[1] - info.MinBounds[1]) * 1.0f / info.SizeY,
                (info.MaxBounds[2] - info.MinBounds[2]) * 1.0f / info.SizeZ);

            const auto sectionTransform = sectionScale * sectionHvaTransform * sectionTranslation;

            for (size_t x = 0; x < info.SizeX; ++x)
            {
                for (size_t y = 0; y < info.SizeY; ++y)
                {
                    for (size_t z = 0; z < info.SizeZ; ++z)
                    {
                        const auto& voxel = body.SpanData[x * info.SizeY + y].Voxels[z];
                        XMVECTOR position = XMVectorSet(x, y, z, 0.0f);
                        position = XMVector3Transform(position, sectionTransform);

                        uint8_t colorIndex = vpl 
                            ? vpl->GetPaletteIndex(normalIndexToVplPage[voxel.Color], voxel.Color) 
                            : voxel.Color;

                        if (colorIndex == 0)
                            continue;

                        RGBStruct color = (remap && colorIndex < 16 || colorIndex > 31)
                            ? RGBStruct{ 255, 0, 255 }
                            : convert[colorIndex];

                        RenderVoxel(position, color, vertexData.size(), vertexIndices, verticesArray);
                        vertexData.append_range(verticesArray);
                    }
                }
            }
        
            RECT sectionImageBounds = GetSectionBounds(info.SizeX, info.SizeY, info.SizeZ, world, sectionTransform);
            UnionRect(&imageBounds, &imageBounds, &sectionImageBounds);
        }

        if (vertexData.empty())
            return { 0,0 };

        {
            const auto width = imageBounds.right - imageBounds.left;
            const auto height = imageBounds.bottom - imageBounds.top;
            const auto x = imageBounds.left - width / 2;
            const auto y = imageBounds.top - height / 2;
            const auto right = x + width * 2;
            const auto bottom = y + height * 2;
            imageBounds.left = x;
            imageBounds.top = y;
            imageBounds.right = right;
            imageBounds.bottom = bottom;
        }

        // var renderTarget = new RenderTarget2D(graphicsDevice, Convert.ToInt32(imageBounds.Width / ModelScale), Convert.ToInt32(imageBounds.Height / ModelScale), false, SurfaceFormat.Color, DepthFormat.Depth24);
        D3D11_TEXTURE2D_DESC desc{};
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.Width = static_cast<UINT>((imageBounds.right - imageBounds.left) / ModelScale);
        desc.Height = static_cast<UINT>((imageBounds.bottom - imageBounds.top) / ModelScale);
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        desc.MiscFlags = 0;

        CComPtr<ID3D11Texture2D> tex2d;
        if (FAILED(device->CreateTexture2D(&desc, nullptr, &tex2d)))
            return { 0,0 };

        D3D11_RENDER_TARGET_VIEW_DESC rtDesc{};
        rtDesc.Format = desc.Format;
        rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        rtDesc.Texture2D.MipSlice = 0;

        CComPtr<ID3D11DeviceContext> context;
        device->GetImmediateContext(&context);

        // Store old render targets
        CComPtr<ID3D11RenderTargetView> oldRT;
        CComPtr<ID3D11DepthStencilView> oldDS;
        context->OMGetRenderTargets(1, &oldRT, &oldDS);

        // Create and use the vxl render target view
        CComPtr<ID3D11RenderTargetView> textureRT;
        if (FAILED(device->CreateRenderTargetView(tex2d, &rtDesc, &textureRT)))
            return { 0,0 };

        D3D11_TEXTURE2D_DESC descDepth = {};
        descDepth.Width = desc.Width;
        descDepth.Height = desc.Height;
        descDepth.MipLevels = 1;
        descDepth.ArraySize = 1;
        descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        descDepth.SampleDesc.Count = 1;
        descDepth.SampleDesc.Quality = 0;
        descDepth.Usage = D3D11_USAGE_DEFAULT;
        descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        descDepth.CPUAccessFlags = 0;
        descDepth.MiscFlags = 0;

        CComPtr<ID3D11Texture2D> depthStencil;
        if (FAILED(device->CreateTexture2D(&descDepth, nullptr, &depthStencil)))
            return { 0,0 };

        // Create depth stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
        descDSV.Format = descDepth.Format;
        descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        descDSV.Texture2D.MipSlice = 0;

        CComPtr<ID3D11DepthStencilView> depthView;
        ;
        if (FAILED(device->CreateDepthStencilView(depthStencil, &descDSV, &depthView)))
            return { 0,0 };

        context->OMSetRenderTargets(1, &textureRT.p, depthView);

        context->ClearRenderTargetView(textureRT, Colors::Transparent);

        XMMATRIX projection = XMMatrixOrthographicRH(
            imageBounds.right - imageBounds.left, 
            imageBounds.bottom - imageBounds.top, 0.1f, 100.0f);

        

        // Restore old render targets and release the temp target view
        context->OMSetRenderTargets(1, &oldRT.p, oldDS);
        textureRT.Release();
    }

}
