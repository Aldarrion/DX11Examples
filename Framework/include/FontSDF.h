#pragma once

#include "Texture.h"
#include "PointWrapSampler.h"
#include "LinearSampler.h"
#include "ShaderProgram.h"
#include "Quad.h"

#include <unordered_map>

namespace Text {

struct SDFCbuffer {
    DirectX::XMMATRIX Model;
    DirectX::XMFLOAT4 UVMul;
};

class FontSDF {
public:
    HRESULT load(const ContextWrapper& context, const std::string& description);
    ~FontSDF();

    bool reloadShaders(ID3D11Device* device);
    DirectX::XMFLOAT4 getUV(char c) const;

    void render(const ContextWrapper& context, const std::string& text, DirectX::XMFLOAT2 position, float size);

private:
    using SDFFontShader_t = ShaderProgram<SDFCbuffer>;
    using PSDFFontShader_t = std::unique_ptr<SDFFontShader_t>;
    using Sampler_t = LinearSampler;

    std::unique_ptr<Sampler_t> sampler_;
    PSDFFontShader_t sdfShader_;
    std::unique_ptr<Quad> quad_;

    ID3D11ShaderResourceView* texture_{};
    ID3D11Texture2D* textureResource_{};

    std::unordered_map<char, DirectX::XMFLOAT2> charCoords_;
    float glyphSize_;
    DirectX::XMFLOAT2 uvPerGlyph_;
};

}
