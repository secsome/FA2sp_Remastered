#pragma once

#include <d3d11.h>
#include <d2d1.h>
#include <dwrite.h>
#include <atlbase.h>

// The class really render the thing
class GameScene
{
public:
	explicit GameScene() = default;
	~GameScene() = default;
	GameScene(const GameScene&) = delete;
	GameScene(GameScene&&) = delete;
	GameScene& operator=(const GameScene&) = delete;
	GameScene& operator=(GameScene&&) = delete;

	HRESULT Initialize(HWND hwnd);
	HRESULT OnSize(int width, int height);

	void Render();

private:
	HRESULT InitDirect3D();
	HRESULT InitDirect2D(); // Requires InitDirect3D called

private:
	HWND hWnd{};
	D3D_DRIVER_TYPE m_driverType{};
	D3D_FEATURE_LEVEL m_featureLevel{};
	CComPtr<ID3D11Device> m_d3dDevice;
	CComPtr<ID3D11DeviceContext> m_d3dDeviceContext;
	CComPtr<IDXGISwapChain> m_dxSwapChain;
	CComPtr<ID3D11RenderTargetView> m_d3dRenderTargetView;
	CComPtr<ID3D11Texture2D> m_d3dDepthStencil;
	CComPtr<ID3D11DepthStencilView> m_d3dDepthStencilView;
	CComPtr<ID2D1Factory> m_d2dFactory;
	CComPtr<ID2D1RenderTarget> m_d2dRenderTarget;
	CComPtr<IDWriteFactory> m_dwriteFactory;
	CComPtr<IDWriteTextFormat> m_dwriteTextFormat;
};