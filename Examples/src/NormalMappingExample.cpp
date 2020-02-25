#include "NormalMappingExample.h"
#include "Transform.h"
#include "PlaneTangentBitangent.h"

namespace NormalMap {
using namespace DirectX;

HRESULT NormalMappingExample::setup() {
    auto hr = BaseExample::setup();
    if (FAILED(hr))
        return hr;

    cube_ = std::make_unique<TexturedCube>(context_.d3dDevice_);
    plane_ = std::make_unique<PlaneTangentBitangent>(context_.d3dDevice_);

    hr = reloadShaders();
    if (FAILED(hr))
        return hr;

    wallDiffuse_ = std::make_unique<Texture>(context_.d3dDevice_, context_.immediateContext_, L"textures/brickwall.dds", true);
    wallNormalMap_ = std::make_unique<Texture>(context_.d3dDevice_, context_.immediateContext_, L"textures/brickwall_normal.dds", false);

    anisoSampler_ = Samplers::createAnisoSampler(context_);

    camera_.positionCamera(
        XMFLOAT3(7.65f, -0.1f, -8.7f),
        XMFLOAT3(0.0f, 1.0f, 0.0f),
        -66.9f,
        -6.7f
    );

    Text::makeDefaultSDFFont(context_, font_);
    using std::to_string;
    infoText_ = std::make_unique<Text::TextSDF>("\n " + to_string(toggleRotationKey_) + ": to toggle rotation.", &font_);

    return hr;
}

bool NormalMappingExample::reloadShadersInternal() {
    return 
        Shaders::makeShader<NormalMapShader>(normalMapShader_, context_.d3dDevice_, "shaders/NormalMap.fx", "VS", "shaders/NormalMap.fx", "PS", plane_->getVertexLayout())
        && Shaders::makeSolidShader(solidShader_, context_);
}

void NormalMappingExample::handleInput() {
    BaseExample::handleInput();

    if (GetAsyncKeyState(toggleRotationKey_) & 1) {
        isRotationOn_ = !isRotationOn_;
    }
}

void NormalMappingExample::render() {
    BaseExample::render();

    const std::array<Transform, NUM_LIGHTS> plTrans = {
        Transform(XMFLOAT3(5.0f, 2.0f, -0.0f), XMFLOAT3(0, 0, 0), XMFLOAT3(0.2f, 0.2f, 0.2f)),
        Transform(XMFLOAT3(0.0f, 2.0f, -0.0f), XMFLOAT3(0, 0, 0), XMFLOAT3(0.2f, 0.2f, 0.2f))
    };
    constexpr float lightIntensity = 1.0f;
    const XMFLOAT4 lightColor = XMFLOAT4(lightIntensity, lightIntensity, lightIntensity, 1);

    if (isRotationOn_) {
        rotation_ += deltaTime_ * XMConvertToRadians(20.0f);
    }

    const std::vector<Transform> cubeTransforms = {
        Transform(XMFLOAT3(0, 0, 0),     XMFLOAT3(XMConvertToRadians(20.0f), XMConvertToRadians(2.0f), rotation_), XMFLOAT3(2, 2, 2)),
        Transform(XMFLOAT3(5.0f, 0, 0),  XMFLOAT3(XMConvertToRadians(20.0f), XMConvertToRadians(2.0f), rotation_), XMFLOAT3(2, 2, 2)),
        Transform(XMFLOAT3(10.0f, 0, 0), XMFLOAT3(XMConvertToRadians(20.0f), XMConvertToRadians(2.0f), rotation_), XMFLOAT3(2, 2, 2)),
        Transform(XMFLOAT3(-5.0f, 0, 0), XMFLOAT3(XMConvertToRadians(20.0f), XMConvertToRadians(2.0f), rotation_), XMFLOAT3(2, 2, 2))
    };

    clearViews();

    // Draw bricks
    {
        NormalMapCB cb{};
        cb.Projection = XMMatrixTranspose(projection_);
        cb.View = XMMatrixTranspose(camera_.getViewMatrix());
        cb.NormalMatrix = computeNormalMatrix(cb.World);
        for (size_t i = 0; i < plTrans.size(); ++i) {
            const auto& light = plTrans[i];
            cb.PointLight[i].Color = lightColor;
            cb.PointLight[i].Position = XMFLOAT4(light.Position.x, light.Position.y, light.Position.z, 1);
        }
        cb.ViewPos = camera_.Position;

        normalMapShader_->use(context_.immediateContext_);
        wallDiffuse_->use(context_.immediateContext_, 0);
        wallNormalMap_->use(context_.immediateContext_, 1);
        anisoSampler_->use(context_.immediateContext_, 0);

        for (size_t i = 0; i < cubeTransforms.size(); ++i) {
            cb.UseNormalMap = i & 1;
            cb.World = XMMatrixTranspose(cubeTransforms[i].generateModelMatrix());
            cb.NormalMatrix = computeNormalMatrix(cb.World);
            normalMapShader_->updateConstantBuffer(context_.immediateContext_, cb);
            //cube_->draw(context_.immediateContext_);
            plane_->draw(context_.immediateContext_);
        }
    }

    // Draw lights
    {
        ConstantBuffers::SolidConstBuffer cb{};
        cb.View = XMMatrixTranspose(camera_.getViewMatrix());
        cb.Projection = XMMatrixTranspose(projection_);
        solidShader_->use(context_.immediateContext_);

        for (const auto& light : plTrans) {
            cb.World = XMMatrixTranspose(light.generateModelMatrix());
            cb.OutputColor = lightColor;
            solidShader_->updateConstantBuffer(context_.immediateContext_, cb);

            cube_->draw(context_.immediateContext_);
        }
    }

    infoText_->draw(context_);

    context_.swapChain_->Present(0, 0);
}
}
