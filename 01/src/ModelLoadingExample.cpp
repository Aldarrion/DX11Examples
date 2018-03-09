#include "ModelLoadingExample.h"
#include "Transform.h"

namespace ModelLoading {

using namespace DirectX;

HRESULT ModelLoadingExample::setup() {
    BaseExample::setup();
    //std::string path = "models/rock/rock.obj";
    //std::string path = "models/box.obj";
    std::string path = "models/nanosuit/nanosuit.obj";

    model_ = std::make_unique<Models::Model>(context_, path);
    shader_ = std::make_unique<ShaderProgram<SpecularCB>>(context_.d3dDevice_, L"shaders/ModelPhong.fx", "VS", L"shaders/ModelPhong.fx", "PS", Layouts::POS_NORM_UV_LAYOUT);
    sampler_ = Samplers::createAnisoSampler(context_);

    return S_OK;
}

void ModelLoadingExample::render() {
    BaseExample::render();

    const Transform sunTrans = Transform(XMFLOAT3(-10.0f, 10.0f, -10.0f), XMFLOAT3(0, 0, 0), XMFLOAT3(0.2f, 0.2f, 0.2f));
    const XMFLOAT4 lightColor = XMFLOAT4(1, 1, 1, 1);
    const Transform modelTransform = Transform(XMFLOAT3(0, -10, 0), XMFLOAT3(0, XMConvertToRadians(180), 0));

    clearViews();

    SpecularCB cb;
    cb.Projection = XMMatrixTranspose(projection_);
    cb.View = XMMatrixTranspose(camera_.getViewMatrix());
    cb.World = XMMatrixTranspose(modelTransform.generateModelMatrix());
    cb.NormalMatrix = computeNormalMatrix(cb.World);
    cb.SunLight.Color = lightColor;
    cb.SunLight.Direction = XMFLOAT4(-sunTrans.Position.x, -sunTrans.Position.y, -sunTrans.Position.z, 1.0f);
    cb.ViewPos = camera_.Position;
    
    sampler_->use(context_.immediateContext_, 0);
    shader_->updateConstantBuffer(context_.immediateContext_, cb);
    shader_->use(context_.immediateContext_);
    model_->draw(context_.immediateContext_);

    context_.swapChain_->Present(0, 0);
}
}
