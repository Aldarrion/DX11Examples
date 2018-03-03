#pragma once
#include "BaseExample.h"
#include "Text.h"

namespace FontRendering {
class FontRenderingExample : public BaseExample {
protected:
    std::unique_ptr<Text::Text> text_;
    std::unique_ptr<Text::Text> fpsText_;
    
    
    HRESULT setup() override;
    void render() override;

public:
    virtual ~FontRenderingExample() = default;
};
}
