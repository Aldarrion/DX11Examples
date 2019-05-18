#pragma once

#include "BaseExample.h"

namespace SDF {

class SignedDistanceFieldFontExample : public BaseExample {
protected:
    HRESULT setup() override;
    void handleInput() override;
    void render() override;
};

}
