#pragma once

#define COM_NO_WINDOWS_H
#include <d3d11_1.h>

struct ContextSettings {
    int MultisampleCount{ 8 };
};

struct ContextWrapper {
    HINSTANCE               hInst_ = nullptr;
    HWND                    hWnd_ = nullptr;
    D3D_DRIVER_TYPE         driverType_ = D3D_DRIVER_TYPE_NULL;
    D3D_FEATURE_LEVEL       featureLevel_ = D3D_FEATURE_LEVEL_11_0;
    ID3D11Device*           d3dDevice_ = nullptr;
    ID3D11DeviceContext*    immediateContext_ = nullptr;
    IDXGISwapChain*         swapChain_ = nullptr;
    ID3D11RenderTargetView* renderTargetView_ = nullptr;
    ID3D11Texture2D*        depthStencil_ = nullptr;
    ID3D11DepthStencilView* depthStencilView_ = nullptr;
    D3D11_VIEWPORT viewPort_;

    int WIDTH = 1280;
    int HEIGHT = 720;

    float getAspectRatio() const;

    HRESULT init(HINSTANCE hInstance, int nCmdShow, const ContextSettings& settings);

    ~ContextWrapper();

    HRESULT enableBlending(bool enable) const;
    HRESULT enableDepthTest(bool enable) const;

private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    HRESULT initWindow(HINSTANCE hInstance, int nCmdShow, const ContextSettings& settings);
    HRESULT initDevice(const ContextSettings& settings);

    void cleanupDevice();
};
