#include "Text.h"

namespace Text {

Text::Text(ID3D11Device* device, ID3D11DeviceContext* context, const std::string& text)
    : text_(text)
    , quad_(device)
    , fontShader_(device, L"shaders/FontShader.fx", "VS", L"shaders/FontShader.fx", "PS", Layouts::POS_UV_LAYOUT)
    , font_(makeInconsolata(device, context))
    , sizeMultiplier_(10.0f)
    , position_({ 0, 0 })
    , textColor_(1, 1, 1, 1) {
}

void Text::setText(const std::string& newText) {
    text_ = newText;
}

void Text::setSize(float size) {
    sizeMultiplier_ = size;
}

void Text::setColor(const DirectX::XMFLOAT4& color) {
    textColor_ = color;
}

void Text::setPosition(const DirectX::XMFLOAT2& position) {
    position_ = position;
}

void Text::setLineSpacing(const float lineSpacing) {
    lineSpacing_ = lineSpacing;
}

float Text::getAbsoluteWidth() const {
    return font_.getWidthSizeScale() * sizeMultiplier_;
}

float Text::getAbsoluteHeight() const {
    return font_.getHeightSizeScale() * sizeMultiplier_;
}

void Text::draw(ID3D11DeviceContext* context, const float aspectRatio) {
    using namespace DirectX;
    XMMATRIX aspectCorrection = XMMatrixScalingFromVector({1, aspectRatio, 1});
    aspectCorrection = aspectCorrection * XMMatrixScalingFromVector({1, 1 / font_.getFontAspectRatio(), 1});

    const float finalSizeScale = getAbsoluteWidth();

    font_.use(context);
    fontShader_.use(context);

    int column = 0;
    int row = 0;
    for (const char i : text_) {
        if (i == '\n') {
            column = 0;
            ++row;
            continue;
        }

        XMMATRIX model = aspectCorrection *
            XMMatrixScalingFromVector({finalSizeScale / 2.0f, finalSizeScale / 2.0f, 1});
        model = model * XMMatrixTranslationFromVector({
            // Left + half letter width to align + which column in the text + position specified
            -1.0f + getAbsoluteWidth() / 2.0f + column * getAbsoluteWidth() + position_.x,
            // Top + half letter height to align + which row in the text + position specified
            1.0f - getAbsoluteHeight() / 2.0f - row * (getAbsoluteHeight() + (lineSpacing_ * getAbsoluteHeight())) -
            position_.y,
            0
        });

        GlyphCb cb{};
        cb.Model = XMMatrixTranspose(model);
        cb.UVWH = font_.getUVWH(i);
        cb.TextColor = textColor_;

        fontShader_.updateConstantBuffer(context, cb);

        quad_.draw(context);

        ++column;
    }
}

PText makeText(ID3D11Device* device, ID3D11DeviceContext* context, const std::string& text) {
    return std::make_unique<Text>(device, context, text);
}
}
