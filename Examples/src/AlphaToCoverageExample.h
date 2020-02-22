#pragma once

#include "BaseExample.h"
#include "ShaderProgram.h"
#include "Texture.h"
#include "AnisotropicSampler.h"
#include "Quad.h"
#include "TextSDF.h"

namespace AlphaToCoverage {

enum BlendMode {
    AlphaToCoverage,
    Blend,
    NoBlend,
    COUNT
};

class AlphaToCoverageExample : public BaseExample {
protected:
    using AtoCShader = ShaderProgram<>;

    std::unique_ptr<AtoCShader> shader_;
    std::unique_ptr<Texture> texture_;
    std::unique_ptr<AnisotropicSampler> sampler_;
    std::unique_ptr<Quad> quad_;

    Text::FontSDF font_;
    std::unique_ptr<Text::TextSDF> text_;

    ID3D11BlendState* alphaToCoverageBlendState_;
    ID3D11BlendState* alphaBlendingBlendState_;
    ID3D11BlendState* noBlendingBlendState_;

    BlendMode blendMode_;

    HRESULT setup() override;
    bool reloadShadersInternal() override;
    void handleInput() override;
    void render() override;
};

}
