#include "SignedDistanceFieldFontExample.h"

using namespace DirectX;

namespace SDF {

HRESULT SignedDistanceFieldFontExample::setup() {
    auto result = BaseExample::setup();

    if (!SUCCEEDED(result))
        return result;

    font_.load(context_, "fonts/RobotoMono-Regular");
    if (!SUCCEEDED(result))
        return result;

    result = reloadShaders();
    if (FAILED(result))
        return result;

    {
        auto txt = Text::TextSDF("Hello from signed distance field font.\nEven we only have 32x32 texture for each glyph,\nthe font is capable of rendering very small and even more impressively very large.", &font_);
        txt.setPosition(XMFLOAT2(16, 32));
        texts_.push_back(txt);
    }

    {
        auto txt = Text::TextSDF("Big text", &font_);
        txt.setPosition(XMFLOAT2(90, 360));
        txt.setSize(200);
        texts_.push_back(txt);
    }

    showMouse();

    return result;
}

Mouse::Mode SignedDistanceFieldFontExample::getInitialMouseMode() {
    // Make mouse visible
    return Mouse::MODE_ABSOLUTE;
}

bool SignedDistanceFieldFontExample::reloadShadersInternal() {
    return font_.reloadShaders(context_.d3dDevice_);
}

void SignedDistanceFieldFontExample::handleInput() {
    BaseExample::handleInput();
}


void SignedDistanceFieldFontExample::render() {
    BaseExample::render();

    clearViews();

    for (const Text::TextSDF& text : texts_) {
        text.render(context_);
    }

    context_.swapChain_->Present(0, 0);
}

}