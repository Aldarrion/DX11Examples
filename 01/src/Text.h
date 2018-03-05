#pragma once

#include "Font.h"
#include "ContextWrapper.h"
#include "DrawableObject.h"
#include "Quad.h"
#include "ShaderProgram.h"
#include <string>

namespace Text {

struct GlyphCb {
    DirectX::XMMATRIX Model;
    DirectX::XMFLOAT4 UVWH;
};

class Text {
private:
    std::string text_;
    Quad quad_;
    ShaderProgram<GlyphCb> fontShader_;
    Font font_;
    float sizeMultiplier_ = 1.0f;
    DirectX::XMFLOAT2 position_;

public:
    Text(ID3D11Device* device, ID3D11DeviceContext* context, const std::string& text)
            : text_(text)
            , quad_(device)
            , fontShader_(device, L"shaders/FontShader.fx", "VS", L"shaders/FontShader.fx", "PS", Layouts::POS_UV_LAYOUT)
            , font_(makeInconsolata(device, context))
            , sizeMultiplier_(10.0f)
            , position_({ 0, 0 }) {
    }

    void setText(const std::string& newText) {
        text_ = newText;
    }

    void setSize(float size) {
        sizeMultiplier_ = size;
    }

    void setPosition(const DirectX::XMFLOAT2& position) {
        position_ = position;
    }

    float getAbsoluteWidth() const {
        return font_.getWidthSizeScale() * sizeMultiplier_;
    }

    float getAbsoluteHeight() const {
        return font_.getHeightSizeScale() * sizeMultiplier_;
    }

    void draw(ID3D11DeviceContext* context, float aspectRatio) {
        using namespace DirectX;
        XMMATRIX aspectCorrection = XMMatrixScalingFromVector({ 1, aspectRatio, 1 });
        aspectCorrection = aspectCorrection * XMMatrixScalingFromVector({ 1, 1 / font_.getFontAspectRatio(), 1 });

        float finalSizeScale = getAbsoluteWidth();

        font_.use(context);
        fontShader_.use(context);

        int column = 0;
        int row = 0;
        for (int i = 0; i < text_.size(); ++i) {
            if (text_[i] == '\n') {
                column = 0;
                ++row;
                continue;
            }
            
            XMMATRIX model = aspectCorrection * XMMatrixScalingFromVector({ finalSizeScale / 2.0f, finalSizeScale / 2.0f, 1 });
            model = model * XMMatrixTranslationFromVector({
                // Left + half letter width to align + which column in the text + position specified
                -1.0f + getAbsoluteWidth() / 2.0f + column * getAbsoluteWidth() + position_.x,
                // Top + half letter height to align + which row in the text + position specified
                1.0f - getAbsoluteHeight() / 2.0f - row * getAbsoluteHeight() - position_.y,
                0
            });

            GlyphCb cb;
            cb.Model = XMMatrixTranspose(model);
            cb.UVWH = font_.getUVWH(text_[i]);

            fontShader_.updateConstantBuffer(context, cb);
            
            quad_.draw(context);

            ++column;
        }
    }
};
}
