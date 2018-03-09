#include "PhongShadingExample.h"
#include <directxcolors.h>
#include "WinKeyMap.h"

using namespace DirectX;

HRESULT Phong::PhongShadingExample::setup() {
    BaseExample::setup();

    // Define the input layout
    std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    // Create shaders
	phongShader_ = std::make_unique<PhongShader>(context_.d3dDevice_, L"shaders/Phong.fx", "VS", L"shaders/Phong.fx", "PS", layout);
    solidShader_ = std::make_unique<SolidShader>(context_.d3dDevice_, L"shaders/Solid.fx", "VS", L"shaders/Solid.fx", "PSSolid", layout);

    // Create object to draw
	// We will use this CUBE model to actually render everything
	//   1) the cube in the middle, which is lit
	//   2) the plane under the cube (it's just going to be a big cube)
	//   3) each light will be also rendered as an unlit white cube (with solidShader)
    colorCube_ = std::make_unique<ColorCube>(context_.d3dDevice_);

    // Create info text with hint to render on screen
    infoText_ = std::make_unique<Text::Text>(context_.d3dDevice_, context_.immediateContext_, "");

    return S_OK;
}

void Phong::PhongShadingExample::render() {
    BaseExample::render();

    // Setup our lighting parameters
    XMFLOAT4 sunPosition = XMFLOAT4(-3.0f, 3.0f, -3.0f, 1.0f);
    const XMFLOAT4 sunColor = SUN_YELLOW;

    XMFLOAT4 pointLightPositions[1] = {
        XMFLOAT4(0.0f, 0.0f, -5.0f, 1.0f),
    };
    XMFLOAT4 pointLightColors[1] = {
        XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f)
    };

    XMFLOAT4 spotLightPos = XMFLOAT4(0.0f, 0.0f, -5.0f, 1.0f);
    XMFLOAT4 spotLightColor = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);

    // Rotate the second light around the origin
    const XMMATRIX mRotate = XMMatrixRotationY(-XM_PIDIV2 * timeFromStart);
    XMVECTOR vLightPos = XMLoadFloat4(&pointLightPositions[0]);
    vLightPos = XMVector3Transform(vLightPos, mRotate);
    XMStoreFloat4(&pointLightPositions[0], vLightPos);

    infoText_->setText("\n To adjust specularity press K/L\n Specularity: " + std::to_string(shininess_));

    context_.immediateContext_->ClearRenderTargetView(context_.renderTargetView_, Colors::MidnightBlue);
    context_.immediateContext_->ClearDepthStencilView(context_.depthStencilView_, D3D11_CLEAR_DEPTH, 1.0f, 0);

    infoText_->draw(context_.immediateContext_, context_.getAspectRatio());

    // ==============================
    // Draw scene with phong lighting
    // ==============================
    {
        // Update matrix variables and lighting variables
        ConstantBuffers::PhongCB cb;
        cb.World = XMMatrixIdentity();
        cb.NormalMatrix = computeNormalMatrix(cb.World);
        cb.View = XMMatrixTranspose(camera_.getViewMatrix());
        cb.Projection = XMMatrixTranspose(projection_);
        cb.PointLightCount = 1;
        cb.PointLights[0].Position = pointLightPositions[0];
        cb.PointLights[0].Color = pointLightColors[0];
        cb.DirLightCount = 1;
        cb.DirLights[0].Color = sunColor;
        cb.DirLights[0].Direction = XMFLOAT4(-sunPosition.x, -sunPosition.y, -sunPosition.z, 1.0);
        cb.SpotLightCount = 1;
        cb.SpotLights[0].Position = spotLightPos;
        cb.SpotLights[0].Direction = XMFLOAT4(0.0f, 0.0f, -1.0f, 1.0f);
        cb.SpotLights[0].Color = spotLightColor;
        cb.SpotLights[0].InnerCone = XMFLOAT4(cos(XMConvertToRadians(43.0f)), 0.0f, 0.0f, 0.0f);
        cb.SpotLights[0].OuterCone = XMFLOAT4(cos(XMConvertToRadians(47.0f)), 0.0f, 0.0f, 0.0f);
        cb.ViewPos = camera_.Position;
        cb.Shininess = shininess_;
        phongShader_->updateConstantBuffer(context_.immediateContext_, cb);

        // Render the cube
        phongShader_->use(context_.immediateContext_);
        colorCube_->draw(context_.immediateContext_);

        // Render plane
        XMFLOAT4 planePos = XMFLOAT4(0.0, -2.0f, 0.0f, 1.0f);
        const XMMATRIX planeScale = XMMatrixScaling(20.0f, 0.2f, 20.0f);
        cb.World = XMMatrixTranspose(planeScale * XMMatrixTranslationFromVector(XMLoadFloat4(&planePos)));
        cb.NormalMatrix = computeNormalMatrix(cb.World);

        phongShader_->updateConstantBuffer(context_.immediateContext_, cb);
        colorCube_->draw(context_.immediateContext_);
    }

    // =================
    // Render each light
    // =================
    {
        ConstantBuffers::SolidConstBuffer solidCb;
        solidCb.View = XMMatrixTranspose(camera_.getViewMatrix());
        solidCb.Projection = XMMatrixTranspose(projection_);
        const XMMATRIX lightScale = XMMatrixScaling(0.2f, 0.2f, 0.2f);
        for (int m = 0; m < 1; m++) {
            XMMATRIX lightModelMatrix = XMMatrixTranslationFromVector(XMLoadFloat4(&pointLightPositions[m]));
            
            lightModelMatrix = lightScale * lightModelMatrix;

            // Update the world variable to reflect the current light
            solidCb.World = XMMatrixTranspose(lightModelMatrix);
            solidCb.OutputColor = pointLightColors[m];
            solidShader_->updateConstantBuffer(context_.immediateContext_, solidCb);

            solidShader_->use(context_.immediateContext_);
            colorCube_->draw(context_.immediateContext_);
        }

        // Render "sun"
        XMMATRIX mLight = XMMatrixTranslationFromVector(XMLoadFloat4(&sunPosition));
        mLight = lightScale * mLight;

        // Update the world variable to reflect the current light
        solidCb.World = XMMatrixTranspose(mLight);
        solidCb.OutputColor = sunColor;
        solidShader_->updateConstantBuffer(context_.immediateContext_, solidCb);

        solidShader_->use(context_.immediateContext_);
        colorCube_->draw(context_.immediateContext_);

        // Render spotlight
        mLight = XMMatrixTranslationFromVector(XMLoadFloat4(&spotLightPos));
        mLight = lightScale * mLight;

        // Update the world variable to reflect the current light
        solidCb.World = XMMatrixTranspose(mLight);
        solidCb.OutputColor = spotLightColor;
        solidShader_->updateConstantBuffer(context_.immediateContext_, solidCb);

        colorCube_->draw(context_.immediateContext_);
    }

    context_.swapChain_->Present(0, 0);
}

void Phong::PhongShadingExample::handleInput() {
    BaseExample::handleInput();

    float step = 1.f;

    if (GetAsyncKeyState(WinKeyMap::K) & 1) {
        shininess_ -= step;
    }
    if (GetAsyncKeyState(WinKeyMap::L) & 1) {
        shininess_ += step;
    }

    shininess_ = max(1.f, min(shininess_, 256.0f));
}
