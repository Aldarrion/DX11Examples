#include "ContextWrapper.h"
#include "Logging.h"

float ContextWrapper::getAspectRatio() const {
    return WIDTH / static_cast<float>(HEIGHT);
}

HRESULT ContextWrapper::init(HINSTANCE hInstance, int nCmdShow, const ContextSettings& settings) {
    if (FAILED(initWindow(hInstance, nCmdShow, settings))) {
        ex::log(ex::LogLevel::Error, "Failed to init window");
        return -1;
    }

    if (FAILED(initDevice(settings))) {
        ex::log(ex::LogLevel::Error, "Failed to init wrapper device");
        return -1;
    }
    return 0;
}

ContextWrapper::~ContextWrapper() {
    cleanupDevice();
}

HRESULT ContextWrapper::enableBlending(bool enable) const {
    D3D11_BLEND_DESC blendDesc;
    ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.RenderTarget[0].BlendEnable = enable;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    ID3D11BlendState* blendState;
    auto hr = d3dDevice_->CreateBlendState(&blendDesc, &blendState);
    if (FAILED(hr)) {
        assert(!"Failed to create blend state");
        return hr;
    }

    float bl[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    immediateContext_->OMSetBlendState(blendState, bl, 0xffffffff);

    return S_OK;
}

HRESULT ContextWrapper::enableDepthTest(bool enable) const {
    D3D11_DEPTH_STENCIL_DESC dsDesc;

    // Depth test parameters
    dsDesc.DepthEnable = enable;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

    // Stencil test parameters
    dsDesc.StencilEnable = true;
    dsDesc.StencilReadMask = 0xFF;
    dsDesc.StencilWriteMask = 0xFF;

    // Stencil operations if pixel is front-facing
    dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Stencil operations if pixel is back-facing
    dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Create depth stencil state
    ID3D11DepthStencilState * pDSState;
    auto hr = d3dDevice_->CreateDepthStencilState(&dsDesc, &pDSState);
    if (FAILED(hr)) {
        assert(!"Failed to create depth stencil desc");
        return hr;
    }

    immediateContext_->OMSetDepthStencilState(pDSState, 1);
    return S_OK;
}

LRESULT ContextWrapper::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
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

HRESULT ContextWrapper::initWindow(HINSTANCE hInstance, int nCmdShow, const ContextSettings&) {
    // Register class
    WNDCLASSEX wcex;
    ZeroMemory(&wcex, sizeof(wcex));
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = "TutorialWindowClass";
    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    // Create window
    hInst_ = hInstance;
    RECT rc = { 0, 0, WIDTH, HEIGHT };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    hWnd_ = CreateWindowA("TutorialWindowClass", "Direct3D 11 Examples",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
        nullptr);
    if (!hWnd_)
        return E_FAIL;

    ShowWindow(hWnd_, nCmdShow);

    return S_OK;
}

HRESULT ContextWrapper::initDevice(const ContextSettings& settings) {
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
        D3D_FEATURE_LEVEL_11_0
    };
    const UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        driverType_ = driverTypes[driverTypeIndex];
        hr = D3D11CreateDevice(
            nullptr,
            driverType_,
            nullptr,
            createDeviceFlags,
            featureLevels,
            numFeatureLevels,
            D3D11_SDK_VERSION,
            &d3dDevice_,
            &featureLevel_,
            &immediateContext_
        );

        if (SUCCEEDED(hr))
            break;
    }
    if (FAILED(hr))
        return hr;

    // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
    IDXGIFactory1* dxgiFactory = nullptr;
    {
        IDXGIDevice* dxgiDevice = nullptr;
        hr = d3dDevice_->QueryInterface(IID_PPV_ARGS(&dxgiDevice));
        if (SUCCEEDED(hr))
        {
            IDXGIAdapter* adapter = nullptr;
            hr = dxgiDevice->GetAdapter(&adapter);
            if (SUCCEEDED(hr))
            {
                hr = adapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
                adapter->Release();
            }
            dxgiDevice->Release();
        }
    }
    if (FAILED(hr))
        return hr;

    // Create swap chain
    {
        // DirectX 11.0 systems
        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = 1;
        sd.BufferDesc.Width = width;
        sd.BufferDesc.Height = height;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hWnd_;
        sd.SampleDesc.Count = settings.MultisampleCount;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        hr = dxgiFactory->CreateSwapChain(d3dDevice_, &sd, &swapChain_);
    }

    // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
    dxgiFactory->MakeWindowAssociation(hWnd_, DXGI_MWA_NO_ALT_ENTER);

    dxgiFactory->Release();

    if (FAILED(hr))
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = swapChain_->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
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
    descDepth.SampleDesc.Count = settings.MultisampleCount;
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
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
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
    ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
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
        assert(!"Failed to create blend state");
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
        assert(!"Failed to create rasterizer state");
        return hr;
    }

    immediateContext_->RSSetState(state);

    return S_OK;
}

void ContextWrapper::cleanupDevice() {
    if (immediateContext_) 
        immediateContext_->ClearState();
    if (depthStencil_) 
        depthStencil_->Release();
    if (depthStencilView_) 
        depthStencilView_->Release();
    if (renderTargetView_) 
        renderTargetView_->Release();
    if (swapChain_) 
        swapChain_->Release();
    if (immediateContext_) 
        immediateContext_->Release();
    if (d3dDevice_) 
        d3dDevice_->Release();
}
