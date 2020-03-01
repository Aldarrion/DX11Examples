#include <windows.h>
#include <d3d11_1.h>
#include <directxcolors.h>

using namespace DirectX;

HINSTANCE               g_hInst = nullptr;
HWND                    g_hWnd = nullptr;
D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*           g_pd3dDevice = nullptr;
ID3D11DeviceContext*    g_pImmediateContext = nullptr;
IDXGISwapChain*         g_pSwapChain = nullptr;
ID3D11RenderTargetView* g_pRenderTargetView = nullptr;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
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

HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow ) {
    // Register class
    WNDCLASSEX wcex{};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = "BasicWindowTutorial";
    if(!RegisterClassEx(&wcex))
        return E_FAIL;

    // Create window
    g_hInst = hInstance;
    RECT rc = { 0, 0, 1280, 720 };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    g_hWnd = CreateWindowA("BasicWindowTutorial", "DX11Examples creating a window",
                           WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                           CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
                           nullptr);
    if (!g_hWnd)
        return E_FAIL;

    ShowWindow(g_hWnd, nCmdShow);

    return S_OK;
}

HRESULT CreateSwapChain(IDXGIFactory* dxgiFactory) {
    HRESULT hr = S_OK;

    // Create swap chain
    RECT rc;
    GetClientRect(g_hWnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 1;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = g_hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    hr = dxgiFactory->CreateSwapChain(g_pd3dDevice, &sd, &g_pSwapChain);

    // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
    dxgiFactory->MakeWindowAssociation(g_hWnd, DXGI_MWA_NO_ALT_ENTER);

    if (FAILED(hr))
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    if (FAILED(hr))
        return hr;

    hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
    pBackBuffer->Release();
    if (FAILED(hr))
        return hr;

    g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    g_pImmediateContext->RSSetViewports(1, &vp);

    return hr;
}

IDXGIAdapter* FindBestAdapter(IDXGIFactory* dxgiFactory, UINT createDeviceFlags, D3D_FEATURE_LEVEL* featureLevels, UINT featureLevelCount) {
    int numAdapters = 0;
    IDXGIAdapter* adapter = nullptr;
    size_t bestAdapterMemory = 0;
    IDXGIAdapter* bestAdapter = nullptr;
    D3D_FEATURE_LEVEL adapterFeatureLevel{};
    for (;SUCCEEDED(dxgiFactory->EnumAdapters(numAdapters, &adapter)); ++numAdapters) {
        
        DXGI_ADAPTER_DESC adapterDesc{};
        if (FAILED(adapter->GetDesc(&adapterDesc))) {
            adapter->Release();
            continue;
        }

        // Try to create device just to see if the adapter is viable, do not save the device yet
        if (FAILED(D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, createDeviceFlags,
            featureLevels, featureLevelCount, D3D11_SDK_VERSION, nullptr, &adapterFeatureLevel, nullptr))) {
            adapter->Release();
            // Device not created, adapter not good enough
            continue;
        }

        if (adapterDesc.DedicatedVideoMemory > bestAdapterMemory) {
            if (bestAdapter) {
                bestAdapter->Release();
            }

            // Take the adapter with the largest dedicated memory - it is probably the best one
            bestAdapterMemory = adapterDesc.DedicatedVideoMemory;
            bestAdapter = adapter;
        } else {
            adapter->Release();
        }
    }

    return bestAdapter;
}

HRESULT InitDevice() {
    HRESULT hr = S_OK;

    IDXGIFactory* dxgiFactory = nullptr;
    if (FAILED(hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory)))) {
        return hr;
    }

    UINT createDeviceFlags = 0;
    #ifdef _DEBUG
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };
    UINT featureLevelCount = ARRAYSIZE(featureLevels);

    IDXGIAdapter* bestAdapter = FindBestAdapter(dxgiFactory, createDeviceFlags, featureLevels, featureLevelCount);

    if (FAILED(hr = D3D11CreateDevice(bestAdapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, createDeviceFlags,
        featureLevels, featureLevelCount, D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext))) {
        // Should never happen
        return hr;
    }

    bestAdapter->Release();

    CreateSwapChain(dxgiFactory);
    dxgiFactory->Release();

    return S_OK;
}

HRESULT InitDeviceAlternative() {
    HRESULT hr = S_OK;

    UINT createDeviceFlags = 0;
    #ifdef _DEBUG
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif

    D3D_DRIVER_TYPE driverTypes[] = {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    for(UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++) {
        g_driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                                D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);

        if (hr == E_INVALIDARG) {
            // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
            hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                                    D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);
        }

        if(SUCCEEDED(hr))
            break;
    }
    if(FAILED(hr))
        return hr;

    // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
    IDXGIFactory1* dxgiFactory = nullptr;
    {
        IDXGIDevice* dxgiDevice = nullptr;
        hr = g_pd3dDevice->QueryInterface(IID_PPV_ARGS(&dxgiDevice));
        if (SUCCEEDED(hr)) {
            IDXGIAdapter* adapter = nullptr;
            hr = dxgiDevice->GetAdapter(&adapter);
            if (SUCCEEDED(hr)) {
                hr = adapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
                adapter->Release();
            }
            dxgiDevice->Release();
        }
    }
    if (FAILED(hr))
        return hr;

    // Create swap chain
    IDXGIFactory2* dxgiFactory2 = nullptr;
    hr = dxgiFactory->QueryInterface(IID_PPV_ARGS(&dxgiFactory2));

    CreateSwapChain(dxgiFactory);
    dxgiFactory->Release();

    return S_OK;
}

void Render()
{
    // Just clear the backbuffer
    g_pImmediateContext->ClearRenderTargetView( g_pRenderTargetView, Colors::MidnightBlue );
    g_pSwapChain->Present( 0, 0 );
}

void CleanupDevice()
{
    if(g_pImmediateContext) g_pImmediateContext->ClearState();
    if(g_pRenderTargetView) g_pRenderTargetView->Release();
    if(g_pSwapChain) g_pSwapChain->Release();
    if(g_pImmediateContext) g_pImmediateContext->Release();
    if(g_pd3dDevice) g_pd3dDevice->Release();
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
    if (FAILED(InitWindow(hInstance, nCmdShow)))
        return 0;

    if (FAILED(InitDevice())) {
        CleanupDevice();
        return 0;
    }

    // Main message loop
    MSG msg{};
    while (WM_QUIT != msg.message) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            Render();
        }
    }

    CleanupDevice();

    return (int)msg.wParam;
}

