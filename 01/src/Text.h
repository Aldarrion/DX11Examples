#pragma once
#include <string>
#include "ContextWrapper.h"
#include "Texture.h"

namespace Text {
class Text {
private:
    std::string text_;
    Texture fontMap_;

public:
    Text(ID3D11Device* device, ID3D11DeviceContext* context, const std::string& text)
            : text_(text)
            , fontMap_(device, context, L"Inconsolata-10-bold.dds") {
    }

    void draw(ID3D11DeviceContext* context) {
        
    }
};
}
