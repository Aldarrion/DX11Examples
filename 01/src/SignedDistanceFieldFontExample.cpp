#include "SignedDistanceFieldFontExample.h"

using namespace DirectX;

namespace SDF {
HRESULT SignedDistanceFieldFontExample::setup() {
    auto result = BaseExample::setup();

    msdfImage_ = std::make_unique<Texture>(context_.d3dDevice_, context_.immediateContext_, L"textures/sdf_a_12.dds", true);
    sampler_ = std::make_unique<Sampler_t>(context_.d3dDevice_);
    quad_ = std::make_unique<Quad>(context_.d3dDevice_);

    reloadShaders();
    showMouse();

    return result;
}

Mouse::Mode SignedDistanceFieldFontExample::getInitialMouseMode() {
    // Make mouse visible
    return Mouse::MODE_ABSOLUTE;
}

bool SignedDistanceFieldFontExample::reloadShadersInternal() {
    return Shaders::makeShader<SDFFontShader_t>(
        sdfShader_,
        context_.d3dDevice_,
        L"shaders/SDFFont.fx", "VS",
        L"shaders/SDFFont.fx", "PS",
        quad_->getVertexLayout()
    );
}

void SignedDistanceFieldFontExample::handleInput() {
    BaseExample::handleInput();
}

void SignedDistanceFieldFontExample::render() {
    BaseExample::render();

    clearViews();

    const float aspectRatio = context_.getAspectRatio();
    const float scale = 0.2f;
    XMMATRIX aspectCorrection = XMMatrixScalingFromVector({ scale / aspectRatio, scale, scale });


    SDFCbuffer cb{};
    cb.Model = aspectCorrection;

    sdfShader_->use(context_.immediateContext_);
    sdfShader_->updateConstantBuffer(context_.immediateContext_, cb);

    msdfImage_->use(context_.immediateContext_, 0);
    sampler_->use(context_.immediateContext_, 0);

    quad_->draw(context_.immediateContext_);

    context_.swapChain_->Present(0, 0);
}
}