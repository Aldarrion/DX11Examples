#pragma once

#include "BaseExample.h"

#include "TextSDF.h"

namespace SDF {

class SignedDistanceFieldFontExample : public BaseExample {
protected:
    Text::FontSDF font_;
    Text::FontSDF fontMsdf_;
    std::vector<Text::TextSDF> texts_;
    bool isMsdf_{ false };

    HRESULT setup() override;
    DirectX::Mouse::Mode getInitialMouseMode() override;
    bool reloadShadersInternal() override;
    void handleInput() override;
    void render() override;
};

}
