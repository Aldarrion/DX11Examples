#pragma once

#include "BaseExample.h"

#include "Texture.h"
#include "PointWrapSampler.h"
#include "LinearSampler.h"
#include "ShaderProgram.h"
#include "Quad.h"

#include <unordered_map>

namespace SDF {

struct SDFCbuffer {
    DirectX::XMMATRIX Model;
    DirectX::XMFLOAT4 UVMul;
};

class FontSDF {
public:
    FontSDF() = default;
    FontSDF(const char* description);

    DirectX::XMFLOAT4 getUV(char c) const;

private:
    std::unordered_map<char, DirectX::XMFLOAT2> charCoords_;
    float glyphSize_;
    DirectX::XMFLOAT2 uvPerGlyph_;
};

class SignedDistanceFieldFontExample : public BaseExample {
public:
    ~SignedDistanceFieldFontExample() override;
protected:
    using SDFFontShader_t = ShaderProgram<SDFCbuffer>;
    using PSDFFontShader_t = std::unique_ptr<SDFFontShader_t>;
    using Sampler_t = LinearSampler;

    ID3D11ShaderResourceView* texture_{};
    ID3D11Texture2D* textureResource_{};

    //std::unique_ptr<Texture> msdfImage_;
    std::unique_ptr<Sampler_t> sampler_;
    PSDFFontShader_t sdfShader_;
    std::unique_ptr<Quad> quad_;

    FontSDF font_;

    HRESULT setup() override;
    DirectX::Mouse::Mode getInitialMouseMode() override;
    bool reloadShadersInternal() override;
    void handleInput() override;
    void render() override;
};

}
