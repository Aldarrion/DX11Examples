#pragma once
#include <string>
#include "ContextWrapper.h"
#include "Texture.h"
#include "DrawableObject.h"
#include "Quad.h"
#include "ShaderProgram.h"
#include "PointWrapSampler.h"
#include "Font.h"

namespace Text {

struct GlyphCb {
    DirectX::XMMATRIX Model;
    DirectX::XMFLOAT4 UVWH;
};

class Text {
private:
    std::string text_;
    Texture fontMap_;
    PointWrapSampler sampler_;
    Quad quad_;
    ShaderProgram<GlyphCb> fontShader_;
    Font font_;
    float sizeMultiplier_ = 1.0f;
    DirectX::XMFLOAT2 position_;

public:
    Text(ID3D11Device* device, ID3D11DeviceContext* context, const std::string& text)
            : text_(text)
            , fontMap_(device, context, L"textures/Inconsolata-10-bold.dds")
            , sampler_(device)
            , quad_(device)
            , fontShader_(device, L"shaders/FontShader.fx", "VS", L"shaders/FontShader.fx", "PS", Layouts::POS_UV_LAYOUT)
            , font_(makeInconsolata())
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

        fontShader_.use(context);
        fontMap_.use(context, 0);
        sampler_.use(context, 0);

        for (int i = 0; i < text_.size(); ++i) {
            // TODO maybe create row based positioning?
            XMMATRIX model = aspectCorrection * XMMatrixScalingFromVector({ finalSizeScale / 2.0f, finalSizeScale / 2.0f, 1 });
            model = model * XMMatrixTranslationFromVector({ 
                -1.0f + finalSizeScale / 2.0f + i * finalSizeScale + position_.x, 
                1.0f - getAbsoluteHeight() / 2.0f - position_.y,
                0 
            });
            //model = model * XMMatrixTranslationFromVector({ -1.0f + i * finalSizeScale, 0.98f, 0 });

            GlyphCb cb;
            cb.Model = XMMatrixTranspose(model);
            cb.UVWH = font_.getUVWH(text_[i]);

            fontShader_.updateConstantBuffer(context, cb);
            
            quad_.draw(context);
        }
    }
};
}
