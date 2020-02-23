#include "GammaCorrectionExample.h"

#include "WinKeyMap.h"
#include "Util.h"

#include <directxcolors.h>

using namespace DirectX;

namespace GammaCorrection {

std::string makeCaption(bool isGammaCorrectionEnabled) {
    return "\n Image shows physically (not perceptually) smooth gradient\n E to toggle gamma correction \n Gamma correction enabled: " + std::to_string(isGammaCorrectionEnabled);
}

HRESULT GammaCorrectionExample::setup() {
    auto hr = BaseExample::setup();
    if (FAILED(hr))
        return hr;

    quad_ = std::make_unique<Quad>(context_.d3dDevice_);

    hr = reloadShaders();
    if (FAILED(hr))
        return hr;

    Text::makeDefaultSDFFont(context_, font_);
    caption_ = std::make_unique<Text::TextSDF>(makeCaption(isGammaCorrectionEnabled_), &font_);
    caption_->setColor(Util::srgbToLinearVec(Colors::Aqua));

    return hr;
}

bool GammaCorrectionExample::reloadShadersInternal() {
    return Shaders::makeShader<GammaCorrectionShader>(
        shader_,
        context_.d3dDevice_,
        L"shaders/GammaCorrectionShowcase.fx", "VS",
        L"shaders/GammaCorrectionShowcase.fx", "PS",
        quad_->getVertexLayout()
    );
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

    GammaCorrectionCb cb{};
    cb.IsGammaCorrectionEnabled.x = float(isGammaCorrectionEnabled_);

    shader_->use(context_.immediateContext_);
    shader_->updateConstantBuffer(context_.immediateContext_, cb);

    quad_->draw(context_.immediateContext_);

    caption_->draw(context_);

    context_.swapChain_->Present(0, 0);
}

}
