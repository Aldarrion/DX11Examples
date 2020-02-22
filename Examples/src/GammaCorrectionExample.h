#pragma once

#include "BaseExample.h"

#include "ShaderProgram.h"
#include "Quad.h"
#include "TextSDF.h"

namespace GammaCorrection {

struct GammaCorrectionCb {
    DirectX::XMFLOAT4 IsGammaCorrectionEnabled;
};

class GammaCorrectionExample : public BaseExample {
protected:
    using GammaCorrectionShader = ShaderProgram<GammaCorrectionCb>;

    std::unique_ptr<GammaCorrectionShader> shader_;

    std::unique_ptr<Quad> quad_;
    Text::FontSDF font_;
    std::unique_ptr<Text::TextSDF> caption_;

    bool isGammaCorrectionEnabled_{ true };

    HRESULT setup() override;
    bool reloadShadersInternal() override;
    void handleInput() override;
    void render() override;
};

}