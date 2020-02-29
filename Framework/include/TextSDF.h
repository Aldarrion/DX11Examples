#pragma once

#include "FontSDF.h"
#include "ContextWrapper.h"

#include <string>
#include <DirectXMath.h>

namespace Text {

class TextSDF {
public:
    TextSDF() = default;
    TextSDF(std::string text, FontSDF* font);
    void setText(std::string text);
    void setPosition(DirectX::XMFLOAT2 position);
    void setSize(float size);
    void setFont(FontSDF* font);
    void setColor(const DirectX::XMFLOAT4& color);

    void draw(ContextWrapper& context) const;

private:
    FontSDF* font_{ nullptr };

    std::string text_{ "" };
    DirectX::XMFLOAT2 position_{ 0.0f, 0.0f };
    float size_{ 24.0f };
    DirectX::XMFLOAT4 color_{ 1, 1, 1, 1 };
};

}
