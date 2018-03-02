#include "FontRenderingExample.h"

namespace FontRendering {

HRESULT FontRenderingExample::setup() {
    BaseExample::setup();


}

void FontRenderingExample::render() {
    BaseExample::render();

    clearViews();



    context_.swapChain_->Present(0, 0);
}
}
