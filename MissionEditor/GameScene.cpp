#include "StdAfx.h"

#include "GameScene.h"

#include "MapData.h"
#include "variables.h"
#include "functions.h"

#include "ShpFile.h"
#include "TmpFile.h"
#include "VxlFile.h"
#include "VplFile.h"
#include "Convert.h"

#include <DirectXColors.h>
#include <DirectXMath.h>

#ifdef _DEBUG
#include "BitmapImage.h"
#endif

HRESULT GameScene::Initialize(HWND hwnd)
{
    hWnd = hwnd;

    HRESULT hr = S_OK;

    hr = InitDirect3D();
    if (FAILED(hr))
        return hr;

    hr = InitDirect2D();
    if (FAILED(hr))
        return hr;

    m_vpl.reset(new VplFile("voxels.vpl"));
    if (m_vpl == nullptr)
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

    return S_OK;
}

HRESULT GameScene::OnSize(int width, int height)
{
    HRESULT hr = S_OK;

    if (m_dxSwapChain)
    {
        // Release
        m_d3dDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
        m_d3dRenderTargetView.Release();

        hr = ReleaseDirect2DResources();
        if (FAILED(hr))
            return hr;

        m_d2dRenderTarget.Release();

        // Direct3D
        hr = m_dxSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

        if (FAILED(hr))
            return hr;

        CComPtr<ID3D11Texture2D> pBuffer = nullptr;
        hr = m_dxSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBuffer));
        if (FAILED(hr))
            return hr;

        hr = m_d3dDevice->CreateRenderTargetView(pBuffer, nullptr, &m_d3dRenderTargetView);
        if (FAILED(hr))
            return hr;

        m_d3dDeviceContext->OMSetRenderTargets(1, &m_d3dRenderTargetView.p, nullptr);

        D3D11_VIEWPORT vp;
        vp.Width = static_cast<FLOAT>(width);
        vp.Height = static_cast<FLOAT>(height);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        m_d3dDeviceContext->RSSetViewports(1, &vp);

        // Direct2D
        CComPtr<IDXGISurface> dxgiBackBuffer = nullptr;
        hr = m_dxSwapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer));
        if (FAILED(hr))
            return hr;

        D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_DEFAULT,
            D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED)
        );

        hr = m_d2dFactory->CreateDxgiSurfaceRenderTarget(dxgiBackBuffer, &props, &m_d2dRenderTarget);
        if (FAILED(hr))
            return hr;

        hr = CreateDirect2DResources();
        if (FAILED(hr))
            return hr;
    }

    return S_OK;
}

void GameScene::Render()
{
    m_d3dDeviceContext->ClearRenderTargetView(m_d3dRenderTargetView, DirectX::Colors::WhiteSmoke);

    DXGI_SWAP_CHAIN_DESC desc;
    HRESULT hr = m_dxSwapChain->GetDesc(&desc);
    if (FAILED(hr))
    {
        m_dxSwapChain->Present(0, 0);
        return;
    }

    D2D1_SIZE_F targetSize = m_d2dRenderTarget->GetSize();

    m_d2dRenderTarget->BeginDraw();

    CComPtr<ID2D1SolidColorBrush> pLightSlateGrayBrush;
    hr = m_d2dRenderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::LightSlateGray),
        &pLightSlateGrayBrush
    );

    CComPtr<ID2D1SolidColorBrush> pBlackBrush;
    hr = m_d2dRenderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::Black),
        &pBlackBrush
    );

    D2D1_RECT_F rect = D2D1::RectF(
        100.0f,
        100.0f,
        targetSize.width - 100.0f,
        targetSize.height - 100.0f
    );

    m_d2dRenderTarget->FillRectangle(&rect, pLightSlateGrayBrush);

    const auto str = utf8ToUtf16(Map->GetIniFile().GetString("Basic", "Name", "No name"));
    m_d2dRenderTarget->DrawText(
        str.c_str(),
        str.length(),
        m_dwriteTextFormat,
        rect,
        pBlackBrush
    );

    hr = m_d2dRenderTarget->EndDraw();
    
    m_dxSwapChain->Present(0, 0);

#ifdef _DEBUG
    VxlFile vxl{ "mtnk" };

    vxl.LoadTexture(m_d3dDevice);

#endif

}

#ifdef _DEBUG
HRESULT GameScene::DumpConvertImage(ID3D11Texture2D* texture, const ConvertClass& convert, const std::string& filename)
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

    auto src = static_cast<const uint8_t*>(mapped.pData);

    bitmap_image bmp{ desc.Width, desc.Height };

    for (size_t y = 0; y < desc.Height; ++y)
    {
        auto line_ptr = src;
        for (size_t x = 0; x < desc.Width; ++x)
        {
            const auto index = *line_ptr++;
            const auto& [r, g, b] = convert[index];
            bmp.set_pixel(x, y, r, g, b);
        }
        src += mapped.RowPitch;
    }

    context->Unmap(staging, 0);

    std::string path = u8ExePath;
    path += filename;

    bmp.save_image(path.c_str());
    
    return S_OK;
}
HRESULT GameScene::DumpConvertShape(ID3D11Texture2D* texture, const std::string& filename)
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

    auto src = static_cast<const uint8_t*>(mapped.pData);

    bitmap_image bmp{ desc.Width, desc.Height };

    for (size_t y = 0; y < desc.Height; ++y)
    {
        auto line_ptr = src;
        for (size_t x = 0; x < desc.Width; ++x)
        {
            const auto index = *line_ptr++;
            bmp.set_pixel(x, y, 255, 255, 255);
        }
        src += mapped.RowPitch;
    }

    context->Unmap(staging, 0);

    std::string path = u8ExePath;
    path += filename;

    bmp.save_image(path.c_str());

    return S_OK;
}
HRESULT GameScene::DumpDepthImage(ID3D11Texture2D* texture, const std::string& filename)
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

    auto src = static_cast<const uint8_t*>(mapped.pData);

    bitmap_image bmp{ desc.Width, desc.Height };

    for (size_t y = 0; y < desc.Height; ++y)
    {
        auto line_ptr = src;
        for (size_t x = 0; x < desc.Width; ++x)
        {
            const auto depth = *line_ptr++;
            bmp.set_pixel(x, y, depth, depth, depth);
        }
        src += mapped.RowPitch;
    }

    context->Unmap(staging, 0);

    std::string path = u8ExePath;
    path += filename;

    bmp.save_image(path.c_str());

    return S_OK;
}
#endif

HRESULT GameScene::InitDirect3D()
{
    HRESULT hr = S_OK;

    RECT rc;
    GetClientRect(hWnd, &rc);

    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    constexpr auto numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    constexpr auto numFeatureLevels = ARRAYSIZE(featureLevels);

    for (auto driverType : driverTypes)
    {
        m_driverType = driverType;
        hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags,
            featureLevels, numFeatureLevels, D3D11_SDK_VERSION, &m_d3dDevice, &m_featureLevel, &m_d3dDeviceContext);

        if (SUCCEEDED(hr))
            break;
    }
    if (FAILED(hr))
        return hr;

    CComPtr<IDXGIFactory1> dxgiFactory = nullptr;
    {
        CComPtr<IDXGIDevice> dxgiDevice = nullptr;
        hr = m_d3dDevice->QueryInterface(IID_PPV_ARGS(&dxgiDevice));
        if (FAILED(hr))
            return hr;

        CComPtr<IDXGIAdapter> adapter = nullptr;
        hr = dxgiDevice->GetAdapter(&adapter);
        if (FAILED(hr))
            return hr;

        hr = adapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
        if (FAILED(hr))
            return hr;
    }

    // Create swap chain
    {
        DXGI_SWAP_CHAIN_DESC sd = {};
        sd.BufferCount = 1;
        sd.BufferDesc.Width = width;
        sd.BufferDesc.Height = height;
        sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hWnd;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;

        hr = dxgiFactory->CreateSwapChain(m_d3dDevice, &sd, &m_dxSwapChain);

        if (FAILED(hr))
            return hr;
    }

    // Create a render target view
    {
        CComPtr<ID3D11Texture2D> pBackBuffer = nullptr;
        hr = m_dxSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        if (FAILED(hr))
            return hr;

        hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_d3dRenderTargetView);
        if (FAILED(hr))
            return hr;
    }

    // Create depth stencil texture
    {
        D3D11_TEXTURE2D_DESC descDepth = {};
        descDepth.Width = width;
        descDepth.Height = height;
        descDepth.MipLevels = 1;
        descDepth.ArraySize = 1;
        descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        descDepth.SampleDesc.Count = 1;
        descDepth.SampleDesc.Quality = 0;
        descDepth.Usage = D3D11_USAGE_DEFAULT;
        descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        descDepth.CPUAccessFlags = 0;
        descDepth.MiscFlags = 0;
        hr = m_d3dDevice->CreateTexture2D(&descDepth, nullptr, &m_d3dDepthStencil);
        if (FAILED(hr))
            return hr;

        // Create depth stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
        descDSV.Format = descDepth.Format;
        descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        descDSV.Texture2D.MipSlice = 0;
        hr = m_d3dDevice->CreateDepthStencilView(m_d3dDepthStencil, &descDSV, &m_d3dDepthStencilView);
        if (FAILED(hr))
            return hr;
    }

    m_d3dDeviceContext->OMSetRenderTargets(1, &m_d3dRenderTargetView.p, m_d3dDepthStencilView);

    // Setup viewport
    D3D11_VIEWPORT vp;
    vp.Width = static_cast<FLOAT>(width);
    vp.Height = static_cast<FLOAT>(height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    m_d3dDeviceContext->RSSetViewports(1, &vp);

    // Compile shaders
    hr = InitializeSceneShaders();
    if (FAILED(hr))
        return hr;



    return S_OK;
}

HRESULT GameScene::InitDirect2D()
{
    HRESULT hr = S_OK;

    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_d2dFactory);
    if (FAILED(hr))
        return hr;

    // Create a Direct2D render target
    CComPtr<IDXGISurface> dxgiBackBuffer = nullptr;
    hr = m_dxSwapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer));
    if (FAILED(hr))
        return hr;

    D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED)
    );

    hr = m_d2dFactory->CreateDxgiSurfaceRenderTarget(dxgiBackBuffer, &props, &m_d2dRenderTarget);
    if (FAILED(hr))
        return hr;

    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&m_dwriteFactory));
    if (FAILED(hr))
        return hr;

    constexpr const wchar_t* TextFontFamily = L"Segoe UI";
    constexpr const wchar_t* TextLocale = L"en-us";
    hr = m_dwriteFactory->CreateTextFormat(
        TextFontFamily,
        nullptr,
        DWRITE_FONT_WEIGHT_REGULAR,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        72.0f,
        TextLocale,
        &m_dwriteTextFormat
    );
    if (FAILED(hr))
        return hr;

    hr = m_dwriteTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    if (FAILED(hr))
        return hr;

    hr = m_dwriteTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    if (FAILED(hr))
        return hr;

    hr = CreateDirect2DResources();
    if (FAILED(hr))
        return hr;

    return S_OK;
}

HRESULT GameScene::CreateDirect2DResources()
{
    HRESULT hr = S_OK;

    return S_OK;
}

HRESULT GameScene::ReleaseDirect2DResources()
{
    HRESULT hr = S_OK;

    return S_OK;
}

HRESULT GameScene::CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    dwShaderFlags |= D3DCOMPILE_DEBUG;
    dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    std::wstring FullPath = u16ExePath;
    FullPath += szFileName;

    CComPtr<ID3DBlob> pErrorBlob = nullptr;
    hr = D3DCompileFromFile(FullPath.c_str(), nullptr, nullptr, szEntryPoint, szShaderModel,
        dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
    if (FAILED(hr))
    {
        if (pErrorBlob)
            OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
        return hr;
    }

    return S_OK;
}

HRESULT GameScene::InitializeSceneShaders()
{
    HRESULT hr = S_OK;

    // Create shader and related input layout stuffs

    return S_OK;
}
