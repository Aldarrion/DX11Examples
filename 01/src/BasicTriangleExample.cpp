#include "BasicTriangleExample.h"
#include <directxcolors.h>

HRESULT BasicTriangleExample::setup() {
    std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    triangle_ = std::make_unique<Triangle>(context_.d3dDevice_);
    shader_ = std::make_unique<SimpleShader>(context_.d3dDevice_, L"shaders/BasicTriangle.fx", "VS", L"shaders/BasicTriangle.fx", "PS", layout);

    return S_OK;
}

void BasicTriangleExample::render() {
    context_.immediateContext_->ClearRenderTargetView(context_.renderTargetView_, DirectX::Colors::MidnightBlue);
    context_.immediateContext_->ClearDepthStencilView(context_.depthStencilView_, D3D11_CLEAR_DEPTH, 1.0f, 0);

    shader_->use(context_.immediateContext_);
    triangle_->draw(context_.immediateContext_);

    context_.swapChain_->Present(0, 0);
}
