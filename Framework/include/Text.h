#pragma once

#include "Font.h"
#include "Quad.h"
#include "ShaderProgram.h"

#include <string>
#include <DirectXMath.h>

namespace Text {

struct GlyphCb {
    DirectX::XMMATRIX Model;
    DirectX::XMFLOAT4 UVWH;
    DirectX::XMFLOAT4 TextColor;
};

class Text {
private:
    std::string text_;
    Quad quad_;
    ShaderProgram<GlyphCb> fontShader_;
    Font font_;
    DirectX::XMFLOAT4 textColor_;
    DirectX::XMFLOAT2 position_;
    float sizeMultiplier_ = 1.0f;
    float lineSpacing_ = 0.2f;

public:
    Text(ID3D11Device* device, ID3D11DeviceContext* context, const std::string& text);

    void setText(const std::string& newText);
    void setSize(float size);
    void setColor(const DirectX::XMFLOAT4& color);
    void setPosition(const DirectX::XMFLOAT2& position);
    void setLineSpacing(float lineSpacing);

    float getAbsoluteWidth() const;
    float getAbsoluteHeight() const;

    void draw(ID3D11DeviceContext* context, const float aspectRatio);
};

using PText = std::unique_ptr<Text>;
PText makeText(ID3D11Device* device, ID3D11DeviceContext* context, const std::string& text);
}
