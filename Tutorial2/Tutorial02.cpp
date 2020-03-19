#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>

using namespace DirectX;

struct SimpleVertex
{
    XMFLOAT3 Pos;
};


HINSTANCE               g_hInst = nullptr;
HWND                    g_hWnd = nullptr;
D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*           g_pd3dDevice = nullptr;
ID3D11DeviceContext*    g_pImmediateContext = nullptr;
IDXGISwapChain*         g_pSwapChain = nullptr;
ID3D11RenderTargetView* g_pRenderTargetView = nullptr;
ID3D11VertexShader*     g_pVertexShader = nullptr;
ID3D11PixelShader*      g_pPixelShader = nullptr;
ID3D11InputLayout*      g_pVertexLayout = nullptr;
ID3D11Buffer*           g_pVertexBuffer = nullptr;


HRESULT initWindow(HINSTANCE hInstance, int nCmdShow);
HRESULT initDevice();
void cleanupDevice();
void render();

// The entry point for Windows applications
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    // First, try to initialize a Window, this is just Windows stuff
    if (FAILED(initWindow(hInstance, nCmdShow)))
        return 0;

    // Now try to initialize the DirectX device
    if (FAILED(initDevice())) {
        cleanupDevice();
        return 0;
    }

    // Main message loop, again, standard Windows stuff
    MSG msg{};
    while (WM_QUIT != msg.message) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            // When all messages are processed call our custom rendering function is called
            render();
        }
    }

    // Don't forget to release all the resources we acquired
    cleanupDevice();

    return (int)msg.wParam;
}

LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
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

// Creates a Windows window which we render to.
HRESULT initWindow(HINSTANCE hInstance, int nCmdShow) {
    // Register class
    WNDCLASSEX wcex{};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = wndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = "BasicWindowTutorial";
    if (!RegisterClassEx(&wcex))
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

HRESULT CompileShaderFromFile(const LPCWSTR szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut) {
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
    #ifdef _DEBUG
        // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
        // Setting this flag improves the shader debugging experience, but still allows 
        // the shaders to be optimized and to run exactly the way they will run in 
        // the release configuration of this program.
        dwShaderFlags |= D3DCOMPILE_DEBUG;

        // Disable optimizations to further improve shader debugging
        dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
    #endif

    ID3DBlob* pErrorBlob = nullptr;
    hr = D3DCompileFromFile( szFileName, nullptr, nullptr, szEntryPoint, szShaderModel, dwShaderFlags, 0, ppBlobOut, &pErrorBlob );
    if(FAILED(hr)) {
        if( pErrorBlob ) {
            OutputDebugStringA( reinterpret_cast<const char*>( pErrorBlob->GetBufferPointer() ) );
            pErrorBlob->Release();
        }
        return hr;
    }
    if(pErrorBlob) 
        pErrorBlob->Release();

    return S_OK;
}


HRESULT compileShaders() {
    // Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr;
    HRESULT hr = CompileShaderFromFile(L"Tutorial02.fx", "VS", "vs_4_0", &pVSBlob);
    if (FAILED(hr)) {
        MessageBoxA(nullptr, "The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", "Error", MB_OK);
        return hr;
    }

    // Create the vertex shader
    hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pVertexShader);
    if (FAILED(hr)) {
        pVSBlob->Release();
        return hr;
    }

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT numElements = ARRAYSIZE(layout);

    // Create the input layout
    hr = g_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
        pVSBlob->GetBufferSize(), &g_pVertexLayout);
    pVSBlob->Release();
    if (FAILED(hr))
        return hr;

    // Set the input layout
    g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

    // Compile the pixel shader
    ID3DBlob* pPSBlob = nullptr;
    hr = CompileShaderFromFile(L"Tutorial02.fx", "PS", "ps_4_0", &pPSBlob);
    if (FAILED(hr)) {
        MessageBoxA(nullptr,
            "The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", "Error", MB_OK);
        return hr;
    }

    // Create the pixel shader
    hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pPixelShader);
    pPSBlob->Release();
    if (FAILED(hr))
        return hr;

    // Create vertex buffer
    SimpleVertex vertices[] =
    {
        XMFLOAT3(0.0f, 0.5f, 0.5f),
        XMFLOAT3(0.5f, -0.5f, 0.5f),
        XMFLOAT3(-0.5f, -0.5f, 0.5f),
    };
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * 3;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices;
    hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);
    if (FAILED(hr))
        return hr;

    // Set vertex buffer
    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;
    g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

    // Set primitive topology
    g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    return S_OK;
}

/* 
 * Finds an adapter which corresponds to given argumens
 * If multiple adapters are found the one with the most dedicated memory is returned
 */
IDXGIAdapter* findBestAdapter(IDXGIFactory* dxgiFactory, UINT createDeviceFlags, D3D_FEATURE_LEVEL* featureLevels, UINT featureLevelCount) {
    IDXGIAdapter* adapter = nullptr;
    size_t bestAdapterMemory = 0;
    IDXGIAdapter* bestAdapter = nullptr;
    D3D_FEATURE_LEVEL adapterFeatureLevel{};

    int numAdapters = 0;
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

// Creates the swapchain for presentation with given DXGIFactory
HRESULT createSwapChain(IDXGIFactory* dxgiFactory) {
    HRESULT hr = S_OK;

    // Get a rectangle corresponding to the window so we can crete a swapchain buffers with the appropriate size
    RECT rc;
    GetClientRect(g_hWnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    // Create the most simple description of the swapchain
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

    if (FAILED(hr))
        return hr;

    // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
    hr = dxgiFactory->MakeWindowAssociation(g_hWnd, DXGI_MWA_NO_ALT_ENTER);

    if (FAILED(hr))
        return hr;

    // The swapchain we created has a backbuffer with the format we specified, take it here
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    if (FAILED(hr))
        return hr;

    // From the swapchain's backbuffer create a Render target view so we can set and clear it
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

// Acquire and initialize all the DirectX11 resources
HRESULT initDevice() {
    HRESULT hr = S_OK;

    // DXGI factory is used to find adapters and create a swapchain
    IDXGIFactory* dxgiFactory = nullptr;
    if (FAILED(hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory)))) {
        return hr;
    }

    UINT createDeviceFlags = 0;
    #ifdef _DEBUG
        // The debug flag enables validation for the DX calls
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif

    // Feature levels do not match the DX runtime version, 11_0 does not mean DX 11.1
    // for more info see the wiki https://en.wikipedia.org/wiki/Feature_levels_in_Direct3D
    // We currently build shaders in SM 5.0 therefore we require FL at least 11.0
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };
    UINT featureLevelCount = ARRAYSIZE(featureLevels);

    // Try to find the best adapter in the system which corresponds to our settings
    IDXGIAdapter* bestAdapter = findBestAdapter(dxgiFactory, createDeviceFlags, featureLevels, featureLevelCount);
    if (!bestAdapter) {
        return S_FALSE;
    }

    // Create the DX device with the adapter we found
    if (FAILED(hr = D3D11CreateDevice(bestAdapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, createDeviceFlags,
        featureLevels, featureLevelCount, D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext))) {
        // Should never happen since the options for device creation should be tested already
        return hr;
    }

    // Adapter is no longer needed by us, all objects which need it (device) hold a reference to it
    bestAdapter->Release();

    if (FAILED(hr = createSwapChain(dxgiFactory))) {
        return hr;
    }
    dxgiFactory->Release();

    compileShaders();

    return S_OK;
}

void render() {
    // Just clear the backbuffer with a solid color
    g_pImmediateContext->ClearRenderTargetView( g_pRenderTargetView, Colors::MidnightBlue );

    // Set the vertex and pixel shader used for the draw call
    g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
    g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
    // Draw 3 vertices starting from vertex 0
    g_pImmediateContext->Draw(3, 0);

    // Call present on the swapchain which flips backbuffer and frontbuffer
    // the first zero specifies that we do not want to wait for vertical sync
    // the second zero stands for no flags to the present function.
    g_pSwapChain->Present(0, 0);
}

void cleanupDevice() {
    if(g_pImmediateContext) g_pImmediateContext->ClearState();
    if(g_pVertexBuffer) g_pVertexBuffer->Release();
    if(g_pVertexLayout) g_pVertexLayout->Release();
    if(g_pVertexShader) g_pVertexShader->Release();
    if(g_pPixelShader) g_pPixelShader->Release();
    if(g_pRenderTargetView) g_pRenderTargetView->Release();
    if(g_pSwapChain) g_pSwapChain->Release();
    if(g_pImmediateContext) g_pImmediateContext->Release();
    if(g_pd3dDevice) g_pd3dDevice->Release();
}

