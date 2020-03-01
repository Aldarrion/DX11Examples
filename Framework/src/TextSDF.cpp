#include "TextSDF.h"

using namespace DirectX;

namespace Text {

TextSDF::TextSDF(std::string text, FontSDF* font)
    : text_(std::move(text))
    , font_(font) {
}

void TextSDF::setText(std::string text) {
    text_ = std::move(text);
}

void TextSDF::setPosition(XMFLOAT2 position) {
    position_ = position;
}

void TextSDF::setSize(float size) {
    size_ = size;
}

void TextSDF::setFont(FontSDF* font) {
    font_ = font;
}

void TextSDF::setColor(const DirectX::XMFLOAT4& color) {
    color_ = color;
}

void TextSDF::draw(const ContextWrapper& context) const {
    if (font_)
        font_->draw(context, text_, position_, size_, color_);
}

}
