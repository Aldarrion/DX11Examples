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


class TextSDF {
public:
    HRESULT load(const ContextWrapper& context, std::string text, const std::string& font);
    bool reloadShaders(ID3D11Device* device);
    void setText(std::string text);
    void setPosition(DirectX::XMFLOAT2 position);
    void setSize(float size);

    void render(const ContextWrapper& context);

private:
    FontSDF font_;

    std::string text_{ "" };
    DirectX::XMFLOAT2 position_{ 0.0f, 0.0f };
    float size_{ 32.0f };
};


class SignedDistanceFieldFontExample : public BaseExample {
protected:
    HRESULT setup() override;
    DirectX::Mouse::Mode getInitialMouseMode() override;
    bool reloadShadersInternal() override;
    void handleInput() override;
    void render() override;

    TextSDF text_;
};

}
