#pragma once
#include "BaseExample.h"

namespace FontRendering {
class FontRenderingExample : public BaseExample {
protected:
    HRESULT setup() override;
    void render() override;

public:
    virtual ~FontRenderingExample() = default;
};
}
