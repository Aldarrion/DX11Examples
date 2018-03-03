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

public:
    Text(ID3D11Device* device, ID3D11DeviceContext* context, const std::string& text)
            : text_(text)
            , fontMap_(device, context, L"textures/Inconsolata-10-bold.dds")
            , sampler_(device)
            , quad_(device)
            , fontShader_(device, L"shaders/FontShader.fx", "VS", L"shaders/FontShader.fx", "PS", Layouts::POS_UV_LAYOUT)
            , font_(makeInconsolata()) {
    }

    void setText(const std::string& newText) {
        text_ = newText;
    }

    void draw(ID3D11DeviceContext* context, float aspectRatio) {
        using namespace DirectX;
        XMMATRIX aspectCorrection = XMMatrixScalingFromVector({ 1, aspectRatio, 1 });

        float sizeScale = 0.02f;

        fontShader_.use(context);
        fontMap_.use(context, 0);
        sampler_.use(context, 0);

        for (int i = 0; i < text_.size(); ++i) {
            // TODO maybe create row based positioning?
            XMMATRIX model = aspectCorrection * XMMatrixScalingFromVector({ sizeScale, sizeScale, 1 });
            model = model * XMMatrixTranslationFromVector({ -1.0f + (i + 1) * sizeScale + (i * sizeScale), 0.98f, 0 });

            GlyphCb cb;
            cb.Model = XMMatrixTranspose(model);
            cb.UVWH = font_.getUVWH(text_[i]);

            fontShader_.updateConstantBuffer(context, cb);
            
            quad_.draw(context);
        }
    }
};
}
