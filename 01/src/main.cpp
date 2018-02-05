//--------------------------------------------------------------------------------------
// File: Tutorial06.cpp
//
// This application demonstrates simple lighting in the vertex shader
//
// http://msdn.microsoft.com/en-us/library/windows/apps/ff729723.aspx
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

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

using namespace DirectX;

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------
struct SimpleVertex {
    XMFLOAT3 Pos;
    XMFLOAT3 Normal;
    XMFLOAT3 Color;
};

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


//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------

ID3D11Buffer*           g_pVertexBuffer = nullptr;
ID3D11Buffer*           g_pIndexBuffer = nullptr;
ID3D11Buffer*           g_pConstantBuffer = nullptr;
XMMATRIX                g_World;
XMMATRIX                g_Projection;
Camera                  camera(XMFLOAT3(0.0f, 4.0f, -10.0f));

bool shouldExit = false;

std::unique_ptr<Mouse> g_mouse;

std::unique_ptr<ShaderProgram<ConstantBuffer>> g_cubeShader;
std::unique_ptr<ShaderProgram<SolidConstBuffer>> g_solidShader;

ContextWrapper g_context;

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
HRESULT Setup();
void cleanupDevice();
void render();


//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
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
    while (WM_QUIT != msg.message && !shouldExit) {
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

    cleanupDevice();

    return static_cast<int>(msg.wParam);
}


//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
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

    // Create vertex buffer
    SimpleVertex vertices[] = {
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.2f, 1.0f, 0.0f) },
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.2f, 1.0f, 0.0f) },
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.2f, 1.0f, 0.0f) },
        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(1.0f, 0.2f, 0.0f) },

        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT3(0.1f, 1.0f, 0.0f) },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT3(0.1f, 1.0f, 0.0f) },
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT3(0.1f, 1.0f, 0.0f) },
        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT3(0.1f, 1.0f, 0.0f) },

        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 0.0f) },
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 0.0f) },
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 0.0f) },
        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 0.2f, 0.0f) },

        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(0.1f, 1.0f, 0.0f) },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(0.1f, 1.0f, 0.0f) },
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(0.1f, 1.0f, 0.0f) },
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(0.1f, 1.0f, 0.0f) },

        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT3(0.5f, 0.7f, 0.5f) },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT3(0.5f, 0.7f, 0.5f) },
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT3(0.5f, 0.7f, 0.5f) },
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT3(0.1f, 0.7f, 0.5f) },

        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.1f, 1.0f, 0.0f) },
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.1f, 1.0f, 0.0f) },
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.1f, 1.0f, 0.0f) },
        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(1.0f, 0.2f, 0.0f) },
    };

    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * 24;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices;
    auto hr = g_context.g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);
    if (FAILED(hr))
        return hr;

    // Set vertex buffer
    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;
    g_context.g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

    // Create index buffer
    // Create vertex buffer
    WORD indices[] = {
        3,1,0,
        2,1,3,

        6,4,5,
        7,4,6,

        11,9,8,
        10,9,11,

        14,12,13,
        15,12,14,

        19,17,16,
        18,17,19,

        22,20,21,
        23,20,22
    };
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * 36;        // 36 vertices needed for 12 triangles in a triangle list
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    InitData.pSysMem = indices;
    hr = g_context.g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pIndexBuffer);
    if (FAILED(hr))
        return hr;

    // Set index buffer
    g_context.g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    // Set primitive topology
    g_context.g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Create the constant buffer
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(ConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = g_context.g_pd3dDevice->CreateBuffer(&bd, nullptr, &g_pConstantBuffer);
    if (FAILED(hr))
        return hr;

    // Initialize the world matrices
    g_World = XMMatrixIdentity();

    // Initialize the projection matrix
    g_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, g_context.WIDTH / static_cast<FLOAT>(g_context.HEIGHT), 0.01f, 100.0f);

    return S_OK;
}

void cleanupDevice() {
    if (g_pConstantBuffer) g_pConstantBuffer->Release();
    if (g_pVertexBuffer) g_pVertexBuffer->Release();
    if (g_pIndexBuffer) g_pIndexBuffer->Release();
}

void handleInput(float deltaTime) {
    if (GetActiveWindow() != g_context.g_hWnd)
        return;

    if (GetAsyncKeyState(VK_ESCAPE)) {
        shouldExit = true;
    }
    if (GetAsyncKeyState(0x57)) {
        camera.ProcessKeyboard(CameraMovement::FORWARD, deltaTime);
    }
    if (GetAsyncKeyState(0x53)) {
        camera.ProcessKeyboard(CameraMovement::BACKWARD, deltaTime);
    }
    if (GetAsyncKeyState(0x41)) {
        camera.ProcessKeyboard(CameraMovement::LEFT, deltaTime);
    }
    if (GetAsyncKeyState(0x44)) {
        camera.ProcessKeyboard(CameraMovement::RIGHT, deltaTime);
    }
    if (GetAsyncKeyState(0x20)) {
        camera.ProcessKeyboard(CameraMovement::UP, deltaTime);
    }
    if (GetAsyncKeyState(0x11)) {
        camera.ProcessKeyboard(CameraMovement::DOWN, deltaTime);
    }
    auto mouse = g_mouse->GetState();
    if (mouse.positionMode == Mouse::MODE_RELATIVE) {
        camera.ProcessMouseMovement(static_cast<float>(-mouse.x), static_cast<float>(mouse.y));
    }
}

std::chrono::steady_clock::time_point lastFrame = std::chrono::high_resolution_clock::now();
double deltaTime = 0.0;
double timeFromStart = 0.0;

//--------------------------------------------------------------------------------------
// Render a frame
//--------------------------------------------------------------------------------------
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

    //
    // Update matrix variables and lighting variables
    //
    ConstantBuffer cb;
    cb.mWorld = XMMatrixTranspose(g_World);
    cb.mView = XMMatrixTranspose(camera.GetViewMatrix());
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
    cb.vViewPos = camera.Position;
    g_cubeShader->updateConstantBuffer(g_context.g_pImmediateContext, cb);

    //
    // Render the cube
    //
    g_cubeShader->use(g_context.g_pImmediateContext);
    g_context.g_pImmediateContext->DrawIndexed(36, 0, 0);

    //
    // Render each light
    //
    {
        SolidConstBuffer solidCb;
        solidCb.mView = XMMatrixTranspose(camera.GetViewMatrix());
        solidCb.mProjection = XMMatrixTranspose(g_Projection);
        for (int m = 0; m < 1; m++) {
            XMMATRIX mLight = XMMatrixTranslationFromVector(XMLoadFloat4(&pointLightPositions[m]));
            XMMATRIX mLightScale = XMMatrixScaling(0.2f, 0.2f, 0.2f);
            mLight = mLightScale * mLight;

            // Update the world variable to reflect the current light
            solidCb.mWorld = XMMatrixTranspose(mLight);
            solidCb.vOutputColor = pointLightColors[m];
            g_solidShader->updateConstantBuffer(g_context.g_pImmediateContext, solidCb);

            g_solidShader->use(g_context.g_pImmediateContext);
            g_context.g_pImmediateContext->DrawIndexed(36, 0, 0);
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
        g_context.g_pImmediateContext->DrawIndexed(36, 0, 0);
    }

    g_context.g_pSwapChain->Present(0, 0);
}


