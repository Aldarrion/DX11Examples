#include "ShaderProgram.h"
#include "ContextWrapper.h"

#include <windows.h>
#include <Windowsx.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>

#include "Camera.h"
#include "Mouse.h"
#include <memory>
#include <iostream>
#include <chrono>
#include "ColorCube.h"

using namespace DirectX;

// ==========
// Structures
// ==========
struct PointLight {
    XMFLOAT4 Position;
    XMFLOAT4 Color;
};

struct DirLight {
    XMFLOAT4 Direction;
    XMFLOAT4 Color;
};

struct SpotLight {
    XMFLOAT4 Position;
    XMFLOAT4 Direction;
    XMFLOAT4 Color;
    XMFLOAT4 InnerCone;
    XMFLOAT4 OuterCone;
};

struct ConstantBuffer {
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
    PointLight PointLights[2];
    DirLight DirLights[2];
    SpotLight SpotLights[2];
    XMFLOAT3 vViewPos;
    int PointLightCount;
    int DirLightCount;
    int SpotLightCount;
};

struct SolidConstBuffer {
    XMMATRIX mWorld;
    XMMATRIX mView;
    XMMATRIX mProjection;
    XMFLOAT4 vOutputColor;
};


// ================
// Global variables
// ================


XMMATRIX                g_World;
XMMATRIX                g_Projection;
Camera                  g_camera(XMFLOAT3(0.0f, 4.0f, -10.0f));

bool g_shouldExit = false;

std::unique_ptr<Mouse> g_mouse;

std::unique_ptr<ShaderProgram<ConstantBuffer>> g_cubeShader;
std::unique_ptr<ShaderProgram<SolidConstBuffer>> g_solidShader;
std::unique_ptr<ColorCube> g_colorCube;

ContextWrapper g_context;

// ====================
// Forward declarations
// ====================
HRESULT Setup();
void render();

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    AllocConsole();
    FILE *pCin, *pCout, *pCerr;
    freopen_s(&pCin, "conin$", "r", stdin);
    freopen_s(&pCout, "conout$", "w", stdout);
    freopen_s(&pCerr, "conout$", "w", stderr);
    std::cout << "Start" << std::endl;
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    auto hr = g_context.Init(hInstance, nCmdShow);
    if (FAILED(hr)) {
        MessageBox(nullptr, L"Failed to init context wrapper", L"Error", MB_OK);
        return 0;
    }
    
    hr = Setup();
    if (FAILED(hr)) {
        MessageBox(nullptr, L"Failed to init device", L"Error", MB_OK);
        return 0;
    }

    g_mouse = std::make_unique<Mouse>();
    g_mouse->SetWindow(g_context.g_hWnd);
    g_mouse->SetMode(Mouse::MODE_RELATIVE);

    // Main message loop
    MSG msg = { 0 };
    while (WM_QUIT != msg.message && !g_shouldExit) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE | PM_NOYIELD)) {
            switch (msg.message) {
                case WM_INPUT:
                case WM_MOUSEMOVE:
                case WM_LBUTTONDOWN:
                case WM_LBUTTONUP:
                case WM_RBUTTONDOWN:
                case WM_RBUTTONUP:
                case WM_MBUTTONDOWN:
                case WM_MBUTTONUP:
                case WM_MOUSEWHEEL:
                case WM_XBUTTONDOWN:
                case WM_XBUTTONUP:
                case WM_MOUSEHOVER:
                case WM_ACTIVATEAPP:
                    Mouse::ProcessMessage(msg.message, msg.wParam, msg.lParam);
                    break;
                default:
                    break;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            render();
        }
    }

    return static_cast<int>(msg.wParam);
}

HRESULT Setup()
{
    // Define the input layout
    std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    g_cubeShader = std::make_unique<ShaderProgram<ConstantBuffer>>(g_context.g_pd3dDevice, L"shaders/Phong.fx", "VS", L"shaders/Phong.fx", "PS", layout);
    g_solidShader = std::make_unique<ShaderProgram<SolidConstBuffer>>(g_context.g_pd3dDevice, L"shaders/Solid.fx", "VS", L"shaders/Solid.fx", "PSSolid", layout);

    g_colorCube = std::make_unique<ColorCube>(g_context.g_pd3dDevice);

    // Initialize the world matrices
    g_World = XMMatrixIdentity();

    // Initialize the projection matrix
    g_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, g_context.WIDTH / static_cast<FLOAT>(g_context.HEIGHT), 0.01f, 100.0f);

    return S_OK;
}

void handleInput(float deltaTime) {
    if (GetActiveWindow() != g_context.g_hWnd)
        return;

    if (GetAsyncKeyState(VK_ESCAPE)) {
        g_shouldExit = true;
    }
    if (GetAsyncKeyState(0x57)) { // W
        g_camera.ProcessKeyboard(CameraMovement::FORWARD, deltaTime);
    }
    if (GetAsyncKeyState(0x53)) { // S
        g_camera.ProcessKeyboard(CameraMovement::BACKWARD, deltaTime);
    }
    if (GetAsyncKeyState(0x41)) { // A
        g_camera.ProcessKeyboard(CameraMovement::LEFT, deltaTime);
    }
    if (GetAsyncKeyState(0x44)) { // D
        g_camera.ProcessKeyboard(CameraMovement::RIGHT, deltaTime);
    }
    if (GetAsyncKeyState(0x20)) { // Space
        g_camera.ProcessKeyboard(CameraMovement::UP, deltaTime);
    }
    if (GetAsyncKeyState(0x11)) { // Ctrl
        g_camera.ProcessKeyboard(CameraMovement::DOWN, deltaTime);
    }
    auto mouse = g_mouse->GetState();
    if (mouse.positionMode == Mouse::MODE_RELATIVE) {
        g_camera.ProcessMouseMovement(static_cast<float>(-mouse.x), static_cast<float>(mouse.y));
    }
}

std::chrono::steady_clock::time_point lastFrame = std::chrono::high_resolution_clock::now();
double deltaTime = 0.0;
double timeFromStart = 0.0;

// ==============
// Render a frame
// ==============
void render()
{
    // Update our time
    const auto currentFrame = std::chrono::high_resolution_clock::now();
    deltaTime = std::chrono::duration_cast<std::chrono::nanoseconds>(currentFrame - lastFrame).count() / 1000000000.0;
    lastFrame = currentFrame;
    timeFromStart += deltaTime;

    handleInput(static_cast<float>(deltaTime));

    // Rotate cube around the origin
    g_World = XMMatrixRotationY(timeFromStart);
    g_World = XMMatrixIdentity();

    // Setup our lighting parameters
    XMFLOAT4 sunPosition = XMFLOAT4(-3.0f, 3.0f, -3.0f, 1.0f);
    const XMFLOAT4 sunColor = XMFLOAT4(0.992f, 0.772f, 0.075f, 1.0f);
    
    XMFLOAT4 pointLightPositions[1] = {
        XMFLOAT4(0.0f, 0.0f, -5.0f, 1.0f),
    };
    XMFLOAT4 pointLightColors[1] = {
        XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f)
    };

    // Rotate the second light around the origin
    const XMMATRIX mRotate = XMMatrixRotationY(-XM_PIDIV2 * timeFromStart);
    XMVECTOR vLightPos = XMLoadFloat4(&pointLightPositions[0]);
    vLightPos = XMVector3Transform(vLightPos, mRotate);
    XMStoreFloat4(&pointLightPositions[0], vLightPos);

    g_context.g_pImmediateContext->ClearRenderTargetView(g_context.g_pRenderTargetView, Colors::MidnightBlue);
    g_context.g_pImmediateContext->ClearDepthStencilView(g_context.g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    // Update matrix variables and lighting variables
    ConstantBuffer cb;
    cb.mWorld = XMMatrixTranspose(g_World);
    cb.mView = XMMatrixTranspose(g_camera.GetViewMatrix());
    cb.mProjection = XMMatrixTranspose(g_Projection);
    cb.PointLightCount = 1;
    cb.PointLights[0].Position = pointLightPositions[0];
    cb.PointLights[0].Color = pointLightColors[0];
    cb.DirLightCount = 1;
    cb.DirLights[0].Color = sunColor;
    cb.DirLights[0].Direction = XMFLOAT4(-sunPosition.x, -sunPosition.y, -sunPosition.z, 1.0);
    cb.SpotLightCount = 1;
    cb.SpotLights[0].Position = XMFLOAT4(0.0f, 0.0f, -5.0f, 1.0f);
    cb.SpotLights[0].Direction = XMFLOAT4(0.0f, 0.0f, -1.0f, 1.0f);
    cb.SpotLights[0].Color = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    cb.SpotLights[0].InnerCone = XMFLOAT4(cos(XMConvertToRadians(43.0f)), 0.0f, 0.0f, 0.0f);
    cb.SpotLights[0].OuterCone = XMFLOAT4(cos(XMConvertToRadians(47.0f)), 0.0f, 0.0f, 0.0f);
    cb.vViewPos = g_camera.Position;
    g_cubeShader->updateConstantBuffer(g_context.g_pImmediateContext, cb);

    // Render the cube
    g_cubeShader->use(g_context.g_pImmediateContext);
    g_colorCube->draw(g_context.g_pImmediateContext);

    // Render each light
    {
        SolidConstBuffer solidCb;
        solidCb.mView = XMMatrixTranspose(g_camera.GetViewMatrix());
        solidCb.mProjection = XMMatrixTranspose(g_Projection);
        for (int m = 0; m < 1; m++) {
            XMMATRIX mLight = XMMatrixTranslationFromVector(XMLoadFloat4(&pointLightPositions[m]));
            const XMMATRIX mLightScale = XMMatrixScaling(0.2f, 0.2f, 0.2f);
            mLight = mLightScale * mLight;

            // Update the world variable to reflect the current light
            solidCb.mWorld = XMMatrixTranspose(mLight);
            solidCb.vOutputColor = pointLightColors[m];
            g_solidShader->updateConstantBuffer(g_context.g_pImmediateContext, solidCb);

            g_solidShader->use(g_context.g_pImmediateContext);
            g_colorCube->draw(g_context.g_pImmediateContext);
        }

        // Render "sun"
        XMMATRIX mLight = XMMatrixTranslationFromVector(XMLoadFloat4(&sunPosition));
        XMMATRIX mLightScale = XMMatrixScaling(0.2f, 0.2f, 0.2f);
        mLight = mLightScale * mLight;

        // Update the world variable to reflect the current light
        solidCb.mWorld = XMMatrixTranspose(mLight);
        solidCb.vOutputColor = sunColor;
        g_solidShader->updateConstantBuffer(g_context.g_pImmediateContext, solidCb);

        g_solidShader->use(g_context.g_pImmediateContext);
        g_colorCube->draw(g_context.g_pImmediateContext);
    }

    g_context.g_pSwapChain->Present(0, 0);
}


