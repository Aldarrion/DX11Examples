#include "GammaCorrectionExample.h"

#include "WinKeyMap.h"
#include "Util.h"

using namespace DirectX;

namespace GammaCorrection {

std::string makeCaption(bool isGammaCorrectionEnabled) {
    return "\n Image shows physically (not perceptually) smooth gradient\n E to toggle gamma correction \n Gamma correction enabled: " + std::to_string(isGammaCorrectionEnabled);
}

HRESULT GammaCorrectionExample::setup() {
    auto hr = BaseExample::setup();

    quad_ = std::make_unique<Quad>(context_.d3dDevice_);

    shader_ = std::make_unique<GammaCorrectionShader>(
        context_.d3dDevice_,
        L"shaders/GammaCorrectionShowcase.fx", "VS",
        L"shaders/GammaCorrectionShowcase.fx", "PS",
        quad_->getVertexLayout()
    );

    caption_ = std::make_unique<Text::Text>(context_.d3dDevice_, context_.immediateContext_, makeCaption(isGammaCorrectionEnabled_));
    caption_->setColor(Util::srgbToLinearVec(Colors::Aqua));

    return hr;
}

void GammaCorrectionExample::handleInput() {
    BaseExample::handleInput();

    if (GetAsyncKeyState(WinKeyMap::E) & 1) {
        isGammaCorrectionEnabled_ = !isGammaCorrectionEnabled_;
        caption_->setText(makeCaption(isGammaCorrectionEnabled_));
    }
}

void GammaCorrectionExample::render() {
    BaseExample::render();

    clearViews();

    caption_->draw(context_.immediateContext_, context_.getAspectRatio());

    GammaCorrectionCb cb{};
    cb.IsGammaCorrectionEnabled.x = float(isGammaCorrectionEnabled_);

    shader_->use(context_.immediateContext_);
    shader_->updateConstantBuffer(context_.immediateContext_, cb);

    quad_->draw(context_.immediateContext_);

    context_.swapChain_->Present(0, 0);
}

}
