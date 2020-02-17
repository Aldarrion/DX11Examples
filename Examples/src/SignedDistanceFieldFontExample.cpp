#include "SignedDistanceFieldFontExample.h"

#include "WinKeyMap.h"

using namespace DirectX;

namespace SDF {

HRESULT SignedDistanceFieldFontExample::setup() {
    auto result = BaseExample::setup();

    if (!SUCCEEDED(result))
        return result;

    result = font_.load(context_, "fonts/RobotoMono-Regular");
    if (!SUCCEEDED(result))
        return result;

    result = fontMsdf_.load(context_, "fonts/RobotoMono-Regular_msdf");
    if (!SUCCEEDED(result))
        return result;

    result = reloadShaders();
    if (FAILED(result))
        return result;

    {
        auto txt = Text::TextSDF("E: toggle SDF and MSDF rendering. Is MSDF: " + std::to_string(isMsdf_), &font_);
        txt.setPosition(XMFLOAT2(16, 32));
        texts_.push_back(txt);
    }

    {
        auto txt = Text::TextSDF(
            "Hello from signed distance field font.\n"
            "Even though we only have 32x32 texture for each glyph,\n"
            "the technology is capable of rendering relatively small glyphs\n"
            "but more impressively very large glyphs compared to the low source resolution.\n"
            "To get even sharper glyphs we may use more channels in the texture\n"
            "to store multiple distances.\n"
            , &font_);
        txt.setPosition(XMFLOAT2(16, 64 + 32));
        texts_.push_back(txt);
    }

    {
        auto txt = Text::TextSDF("Big text", &font_);
        txt.setPosition(XMFLOAT2(90, 360));
        txt.setSize(256);
        texts_.push_back(txt);
    }

    {
        auto txt = Text::TextSDF(
            "Source (the original Valve paper):\n"
            "Green, Chris.\n"
            "\"Improved alpha - tested magnification for vector textures and special effects.\"\n"
            "ACM SIGGRAPH 2007 courses. ACM, 2007."
            , &font_);
        txt.setPosition(XMFLOAT2(10, 550));
        txt.setSize(20);
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
    return 
        font_.reloadShaders(context_.d3dDevice_) 
        && fontMsdf_.reloadShaders(context_.d3dDevice_);
}

void SignedDistanceFieldFontExample::handleInput() {
    BaseExample::handleInput();

    if (GetAsyncKeyState(WinKeyMap::E) & 1) {
        isMsdf_ = !isMsdf_;

        texts_[0].setText("E: toggle SDF and MSDF rendering. Is MSDF: " + std::to_string(isMsdf_));
        
        if (isMsdf_) {
            for (auto& text : texts_)
                text.setFont(&fontMsdf_);
        } else {
            for (auto& text : texts_)
                text.setFont(&font_);
        }
    }
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