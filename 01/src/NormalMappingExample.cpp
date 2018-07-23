#include "NormalMappingExample.h"
#include "Transform.h"
#include "PlaneTangentBitangent.h"

namespace NormalMap {
using namespace DirectX;

HRESULT NormalMappingExample::setup() {
    auto hr = BaseExample::setup();

    cube_ = std::make_unique<TexturedCube>(context_.d3dDevice_);
    plane_ = std::make_unique<PlaneTangentBitangent>(context_.d3dDevice_);

    normalMapShader_ = std::make_unique<NormalMapShader>(context_.d3dDevice_, L"shaders/NormalMap.fx", "VS", L"shaders/NormalMap.fx", "PS", plane_->getVertexLayout());
    solidShader_ = Shaders::createSolidShader(context_);

    wallDiffuse_ = std::make_unique<Texture>(context_.d3dDevice_, context_.immediateContext_, L"textures/brickwall.dds");
    wallNormalMap_ = std::make_unique<Texture>(context_.d3dDevice_, context_.immediateContext_, L"textures/brickwall_normal.dds");

    anisoSampler_ = Samplers::createAnisoSampler(context_);
    pointWrapSampler_ = Samplers::createPointWrapSampler(context_);

    camera_.positionCamera(
        XMFLOAT3(7.50966454f, -4.39316130f, -6.65817976f),
        XMFLOAT3(0.0f, 0.1f, 0.0f),
        -46.8005867f,
        -25.4000607f
    );

    return hr;
}

void NormalMappingExample::handleInput() {
    BaseExample::handleInput();
}

void NormalMappingExample::render() {
    BaseExample::render();

    const Transform sunTrans = Transform(XMFLOAT3(-10.0f, 10.0f, -10.0f), XMFLOAT3(0, 0, 0), XMFLOAT3(0.2f, 0.2f, 0.2f));
    const XMFLOAT4 lightColor = XMFLOAT4(1, 1, 1, 1);

    const std::vector<Transform> cubeTransforms = {
        Transform(XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0), XMFLOAT3(2, 2, 2)),
        //Transform(XMFLOAT3(2.5f, 0, 0)),
        //Transform(XMFLOAT3(5.f, 0, 0)),
        //Transform(XMFLOAT3(-2.5f, 0, 0))
    };

    clearViews();

    // Draw bricks
    {
        NormalMapCB cb{};
        cb.Projection = XMMatrixTranspose(projection_);
        cb.View = XMMatrixTranspose(camera_.getViewMatrix());
        cb.NormalMatrix = computeNormalMatrix(cb.World);
        cb.SunLight.Color = lightColor;
        cb.SunLight.Direction = XMFLOAT4(-sunTrans.Position.x, -sunTrans.Position.y, -sunTrans.Position.z, 1.0f);
        cb.ViewPos = camera_.Position;

        normalMapShader_->use(context_.immediateContext_);
        wallDiffuse_->use(context_.immediateContext_, 0);
        wallNormalMap_->use(context_.immediateContext_, 1);
        anisoSampler_->use(context_.immediateContext_, 0);
        pointWrapSampler_->use(context_.immediateContext_, 1);

        for (size_t i = 0; i < cubeTransforms.size(); ++i) {
            cb.UseNormalMap = 1; // i & 1
            cb.World = XMMatrixTranspose(cubeTransforms[i].generateModelMatrix());
            cb.NormalMatrix = computeNormalMatrix(cb.World);
            normalMapShader_->updateConstantBuffer(context_.immediateContext_, cb);
            //cube_->draw(context_.immediateContext_);
            plane_->draw(context_.immediateContext_);
        }
    }

    // Draw sun
    {
        ConstantBuffers::SolidConstBuffer cb{};
        cb.World = XMMatrixTranspose(sunTrans.generateModelMatrix());
        cb.View = XMMatrixTranspose(camera_.getViewMatrix());
        cb.Projection = XMMatrixTranspose(projection_);
        cb.OutputColor = lightColor;

        solidShader_->use(context_.immediateContext_);
        solidShader_->updateConstantBuffer(context_.immediateContext_, cb);
        cube_->draw(context_.immediateContext_);
    }

    context_.swapChain_->Present(0, 0);
}
}
