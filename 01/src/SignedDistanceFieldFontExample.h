#pragma once

#include "BaseExample.h"

#include "Texture.h"
#include "PointWrapSampler.h"
#include "LinearSampler.h"
#include "ShaderProgram.h"
#include "Quad.h"

namespace SDF {

struct SDFCbuffer {
    DirectX::XMMATRIX Model;
};

class SignedDistanceFieldFontExample : public BaseExample {
protected:
    using SDFFontShader_t = ShaderProgram<SDFCbuffer>;
    using PSDFFontShader_t = std::unique_ptr<SDFFontShader_t>;
    using Sampler_t = LinearSampler;

    std::unique_ptr<Texture> msdfImage_;
    std::unique_ptr<Sampler_t> sampler_;
    PSDFFontShader_t sdfShader_;
    std::unique_ptr<Quad> quad_;

    HRESULT setup() override;
    DirectX::Mouse::Mode getInitialMouseMode() override;
    bool reloadShadersInternal() override;
    void handleInput() override;
    void render() override;
};

}
