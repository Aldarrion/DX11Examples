#pragma once
#include "BaseExample.h"
#include "TextBitmap.h"

namespace FontRendering {
class FontRenderingExample : public BaseExample {
protected:
    std::unique_ptr<Text::TextBitmap> text_;
    std::unique_ptr<Text::TextBitmap> fpsText_;
    std::unique_ptr<Text::TextBitmap> multilineText_;
    
    HRESULT setup() override;
    void render() override;
};
}
