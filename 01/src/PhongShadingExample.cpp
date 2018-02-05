#include "PhongShadingExample.h"
#include <directxcolors.h>

using namespace DirectX;

HRESULT PhongShadingExample::setup() {
    // Define the input layout
    std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    cubeShader_ = std::make_unique<ShaderProgram<ConstantBuffer>>(context_.g_pd3dDevice, L"shaders/Phong.fx", "VS", L"shaders/Phong.fx", "PS", layout);
    solidShader_ = std::make_unique<ShaderProgram<SolidConstBuffer>>(context_.g_pd3dDevice, L"shaders/Solid.fx", "VS", L"shaders/Solid.fx", "PSSolid", layout);

    colorCube_ = std::make_unique<ColorCube>(context_.g_pd3dDevice);

    // Initialize the world matrices
    world_ = XMMatrixIdentity();

    // Initialize the projection matrix
    projection_ = XMMatrixPerspectiveFovLH(XM_PIDIV4, context_.WIDTH / static_cast<FLOAT>(context_.HEIGHT), 0.01f, 100.0f);

    return S_OK;
}

void PhongShadingExample::render() {
    BasicExample::render();

    // Rotate cube around the origin
    world_ = XMMatrixRotationY(timeFromStart);
    world_ = XMMatrixIdentity();

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

    context_.g_pImmediateContext->ClearRenderTargetView(context_.g_pRenderTargetView, Colors::MidnightBlue);
    context_.g_pImmediateContext->ClearDepthStencilView(context_.g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    // Update matrix variables and lighting variables
    ConstantBuffer cb;
    cb.mWorld = XMMatrixTranspose(world_);
    cb.mView = XMMatrixTranspose(camera_.GetViewMatrix());
    cb.mProjection = XMMatrixTranspose(projection_);
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
    cb.vViewPos = camera_.Position;
    cubeShader_->updateConstantBuffer(context_.g_pImmediateContext, cb);

    // Render the cube
    cubeShader_->use(context_.g_pImmediateContext);
    colorCube_->draw(context_.g_pImmediateContext);

    // Render each light
    {
        SolidConstBuffer solidCb;
        solidCb.mView = XMMatrixTranspose(camera_.GetViewMatrix());
        solidCb.mProjection = XMMatrixTranspose(projection_);
        for (int m = 0; m < 1; m++) {
            XMMATRIX mLight = XMMatrixTranslationFromVector(XMLoadFloat4(&pointLightPositions[m]));
            const XMMATRIX mLightScale = XMMatrixScaling(0.2f, 0.2f, 0.2f);
            mLight = mLightScale * mLight;

            // Update the world variable to reflect the current light
            solidCb.mWorld = XMMatrixTranspose(mLight);
            solidCb.vOutputColor = pointLightColors[m];
            solidShader_->updateConstantBuffer(context_.g_pImmediateContext, solidCb);

            solidShader_->use(context_.g_pImmediateContext);
            colorCube_->draw(context_.g_pImmediateContext);
        }

        // Render "sun"
        XMMATRIX mLight = XMMatrixTranslationFromVector(XMLoadFloat4(&sunPosition));
        XMMATRIX mLightScale = XMMatrixScaling(0.2f, 0.2f, 0.2f);
        mLight = mLightScale * mLight;

        // Update the world variable to reflect the current light
        solidCb.mWorld = XMMatrixTranspose(mLight);
        solidCb.vOutputColor = sunColor;
        solidShader_->updateConstantBuffer(context_.g_pImmediateContext, solidCb);

        solidShader_->use(context_.g_pImmediateContext);
        colorCube_->draw(context_.g_pImmediateContext);
    }

    context_.g_pSwapChain->Present(0, 0);
}
