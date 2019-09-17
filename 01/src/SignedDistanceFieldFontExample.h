#pragma once

#include "BaseExample.h"

#include "TextSDF.h"

namespace SDF {

class SignedDistanceFieldFontExample : public BaseExample {
protected:
    HRESULT setup() override;
    DirectX::Mouse::Mode getInitialMouseMode() override;
    bool reloadShadersInternal() override;
    void handleInput() override;
    void render() override;

    Text::FontSDF font_;
    std::vector<Text::TextSDF> texts_;
};

}
