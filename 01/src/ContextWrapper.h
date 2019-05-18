#pragma once

#include <windows.h>
#include <d3d11_1.h>

struct ContextWrapper {
    HINSTANCE               hInst_ = nullptr;
    HWND                    hWnd_ = nullptr;
    D3D_DRIVER_TYPE         driverType_ = D3D_DRIVER_TYPE_NULL;
    D3D_FEATURE_LEVEL       featureLevel_ = D3D_FEATURE_LEVEL_11_0;
    ID3D11Device*           d3dDevice_ = nullptr;
    ID3D11Device1*          d3dDevice1_ = nullptr;
    ID3D11DeviceContext*    immediateContext_ = nullptr;
    ID3D11DeviceContext1*   immediateContext1_ = nullptr;
    IDXGISwapChain*         swapChain_ = nullptr;
    IDXGISwapChain1*        swapChain1_ = nullptr;
    ID3D11RenderTargetView* renderTargetView_ = nullptr;
    ID3D11Texture2D*        depthStencil_ = nullptr;
    ID3D11DepthStencilView* depthStencilView_ = nullptr;
    D3D11_VIEWPORT viewPort_;

    int WIDTH = 1280;
    int HEIGHT = 720;

    float getAspectRatio() const {
        return WIDTH / static_cast<float>(HEIGHT);
    }

    HRESULT Init(_In_ HINSTANCE hInstance, _In_ int nCmdShow) {
        if (FAILED(InitWindow(hInstance, nCmdShow))) {
            MessageBox(nullptr, L"Failed to window", L"Error", MB_OK);
            return -1;
        }

        if (FAILED(InitDevice())) {
            MessageBox(nullptr, L"Failed to init wrapper device", L"Error", MB_OK);
            return -1;
        }
        return 0;
    }

    ~ContextWrapper() {
        CleanupDevice();
    }

private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        PAINTSTRUCT ps;
        HDC hdc;

        switch (message) {
            case WM_PAINT:
                hdc = BeginPaint(hWnd, &ps);
                EndPaint(hWnd, &ps);
                break;

            case WM_DESTROY:
                PostQuitMessage(0);
                break;

                // Note that this tutorial does not handle resizing (WM_SIZE) requests,
                // so we created the window without the resize border.

            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
        }

        return 0;
    }

    HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow)
    {
        // Register class
        WNDCLASSEX wcex;
        ZeroMemory(&wcex, sizeof wcex);
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = hInstance;
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = L"TutorialWindowClass";
        if (!RegisterClassEx(&wcex))
            return E_FAIL;

        // Create window
        hInst_ = hInstance;
        RECT rc = { 0, 0, WIDTH, HEIGHT };
        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
        hWnd_ = CreateWindow(L"TutorialWindowClass", L"Direct3D 11 Tutorial",
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
            CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
            nullptr);
        if (!hWnd_)
            return E_FAIL;

        ShowWindow(hWnd_, nCmdShow);

        return S_OK;
    }

    HRESULT InitDevice() {
        HRESULT hr = S_OK;

        RECT rc;
        GetClientRect(hWnd_, &rc);
        const UINT width = rc.right - rc.left;
        const UINT height = rc.bottom - rc.top;

        UINT createDeviceFlags = 0;
#ifdef _DEBUG
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        D3D_DRIVER_TYPE driverTypes[] =
        {
            D3D_DRIVER_TYPE_HARDWARE,
            D3D_DRIVER_TYPE_WARP,
            D3D_DRIVER_TYPE_REFERENCE,
        };
        const UINT numDriverTypes = ARRAYSIZE(driverTypes);

        D3D_FEATURE_LEVEL featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };
        const UINT numFeatureLevels = ARRAYSIZE(featureLevels);

        for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
        {
            driverType_ = driverTypes[driverTypeIndex];
            hr = D3D11CreateDevice(nullptr, driverType_, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                D3D11_SDK_VERSION, &d3dDevice_, &featureLevel_, &immediateContext_);

            if (hr == E_INVALIDARG)
            {
                // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
                hr = D3D11CreateDevice(nullptr, driverType_, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                    D3D11_SDK_VERSION, &d3dDevice_, &featureLevel_, &immediateContext_);
            }

            if (SUCCEEDED(hr))
                break;
        }
        if (FAILED(hr))
            return hr;

        // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
        IDXGIFactory1* dxgiFactory = nullptr;
        {
            IDXGIDevice* dxgiDevice = nullptr;
            hr = d3dDevice_->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
            if (SUCCEEDED(hr))
            {
                IDXGIAdapter* adapter = nullptr;
                hr = dxgiDevice->GetAdapter(&adapter);
                if (SUCCEEDED(hr))
                {
                    hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
                    adapter->Release();
                }
                dxgiDevice->Release();
            }
        }
        if (FAILED(hr))
            return hr;

        // Create swap chain
        IDXGIFactory2* dxgiFactory2 = nullptr;
        hr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2));
        if (dxgiFactory2)
        {
            // DirectX 11.1 or later
            hr = d3dDevice_->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&d3dDevice1_));
            if (SUCCEEDED(hr)) {
                (void)immediateContext_->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&immediateContext1_));
            }

            DXGI_SWAP_CHAIN_DESC1 sd;
            ZeroMemory(&sd, sizeof(sd));
            sd.Width = width;
            sd.Height = height;
            sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            sd.SampleDesc.Count = 1;
            sd.SampleDesc.Quality = 0;
            sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            sd.BufferCount = 1;

            hr = dxgiFactory2->CreateSwapChainForHwnd(d3dDevice_, hWnd_, &sd, nullptr, nullptr, &swapChain1_);
            if (SUCCEEDED(hr)) {
                hr = swapChain1_->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&swapChain_));
            }

            dxgiFactory2->Release();
        }
        else
        {
            // DirectX 11.0 systems
            DXGI_SWAP_CHAIN_DESC sd;
            ZeroMemory(&sd, sizeof(sd));
            sd.BufferCount = 1;
            sd.BufferDesc.Width = width;
            sd.BufferDesc.Height = height;
            sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            sd.BufferDesc.RefreshRate.Numerator = 60;
            sd.BufferDesc.RefreshRate.Denominator = 1;
            sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            sd.OutputWindow = hWnd_;
            sd.SampleDesc.Count = 1;
            sd.SampleDesc.Quality = 0;
            sd.Windowed = TRUE;

            hr = dxgiFactory->CreateSwapChain(d3dDevice_, &sd, &swapChain_);
        }

        // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
        dxgiFactory->MakeWindowAssociation(hWnd_, DXGI_MWA_NO_ALT_ENTER);

        dxgiFactory->Release();

        if (FAILED(hr))
            return hr;

        // Create a render target view
        ID3D11Texture2D* pBackBuffer = nullptr;
        hr = swapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
        if (FAILED(hr))
            return hr;

        hr = d3dDevice_->CreateRenderTargetView(pBackBuffer, nullptr, &renderTargetView_);
        pBackBuffer->Release();
        if (FAILED(hr))
            return hr;

        // Create depth stencil texture
        D3D11_TEXTURE2D_DESC descDepth;
        ZeroMemory(&descDepth, sizeof(descDepth));
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
        hr = d3dDevice_->CreateTexture2D(&descDepth, nullptr, &depthStencil_);
        if (FAILED(hr))
            return hr;

        // Create the depth stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
        ZeroMemory(&descDSV, sizeof(descDSV));
        descDSV.Format = descDepth.Format;
        descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        descDSV.Texture2D.MipSlice = 0;
        hr = d3dDevice_->CreateDepthStencilView(depthStencil_, &descDSV, &depthStencilView_);
        if (FAILED(hr))
            return hr;

        immediateContext_->OMSetRenderTargets(1, &renderTargetView_, depthStencilView_);

        // Setup the viewport
        viewPort_.Width = static_cast<FLOAT>(width);
        viewPort_.Height = static_cast<FLOAT>(height);
        viewPort_.MinDepth = 0.0f;
        viewPort_.MaxDepth = 1.0f;
        viewPort_.TopLeftX = 0;
        viewPort_.TopLeftY = 0;
        immediateContext_->RSSetViewports(1, &viewPort_);

        // =====================
        // Enable alpha blending
        // =====================

        D3D11_BLEND_DESC blendDesc;
        ZeroMemory(&blendDesc, sizeof D3D11_BLEND_DESC);
        blendDesc.RenderTarget[0].BlendEnable = true;
        blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        ID3D11BlendState* blendState;
        hr = d3dDevice_->CreateBlendState(&blendDesc, &blendState);
        if (FAILED(hr)) {
            MessageBox(nullptr, L"Failed to create blend state", L"Error", MB_OK);
            return hr;
        }

        float bl[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        immediateContext_->OMSetBlendState(blendState, bl, 0xffffffff);

        // ==================
        // Setup face culling
        // ==================
        
        D3D11_RASTERIZER_DESC CurrentRasterizerState;
        CurrentRasterizerState.FillMode = D3D11_FILL_SOLID;
        CurrentRasterizerState.CullMode = D3D11_CULL_BACK;
        CurrentRasterizerState.FrontCounterClockwise = false;
        CurrentRasterizerState.DepthBias = false;
        CurrentRasterizerState.DepthBiasClamp = 0;
        CurrentRasterizerState.SlopeScaledDepthBias = 0;
        CurrentRasterizerState.DepthClipEnable = true;
        CurrentRasterizerState.ScissorEnable = false;
        CurrentRasterizerState.MultisampleEnable = true;
        CurrentRasterizerState.AntialiasedLineEnable = false;

        ID3D11RasterizerState* state = nullptr;
        hr = d3dDevice_->CreateRasterizerState(&CurrentRasterizerState, &state);
        if (FAILED(hr)) {
            MessageBox(nullptr, L"Failed to create rasterizer state", L"Error", MB_OK);
            return hr;
        }

        immediateContext_->RSSetState(state);

        return S_OK;
    }

    void CleanupDevice() {
        if (immediateContext_) immediateContext_->ClearState();
        if (depthStencil_) depthStencil_->Release();
        if (depthStencilView_) depthStencilView_->Release();
        if (renderTargetView_) renderTargetView_->Release();
        if (swapChain1_) swapChain1_->Release();
        if (swapChain_) swapChain_->Release();
        if (immediateContext1_) immediateContext1_->Release();
        if (immediateContext_) immediateContext_->Release();
        if (d3dDevice1_) d3dDevice1_->Release();
        if (d3dDevice_) d3dDevice_->Release();
    }
};
