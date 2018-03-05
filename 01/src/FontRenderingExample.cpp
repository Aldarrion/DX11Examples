#include "FontRenderingExample.h"

namespace FontRendering {

std::string specString(float spec) {
    return "To adjust specularity press K/L: ";
}

std::string frameTimeString(float time) {
    return "Frame time: " + std::to_string(time);
}

HRESULT FontRenderingExample::setup() {
    BaseExample::setup();

    fpsText_ = std::make_unique<Text::Text>(context_.d3dDevice_, context_.immediateContext_, frameTimeString(0));
    text_ = std::make_unique<Text::Text>(context_.d3dDevice_, context_.immediateContext_, specString(0));
    text_->setPosition({ 0, text_->getAbsoluteHeight() });

    multilineText_ = std::make_unique<Text::Text>(context_.d3dDevice_, context_.immediateContext_, "1. First line...\n2. Second line... :)\n3. etc.");
    multilineText_->setPosition({ 0, text_->getAbsoluteHeight() * 3 });

    return S_OK;
}

void FontRenderingExample::render() {
    BaseExample::render();

    fpsText_->setText(frameTimeString(deltaTime_));
    text_->setText(specString(deltaTime_));

    clearViews();

    float aspectRatio = context_.getAspectRatio();

    fpsText_->draw(context_.immediateContext_, aspectRatio);
    text_->draw(context_.immediateContext_, aspectRatio);
    multilineText_->draw(context_.immediateContext_, aspectRatio);

    context_.swapChain_->Present(0, 0);
}
}
