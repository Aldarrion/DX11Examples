#include "BasicTriangleExample.h"
#include <directxcolors.h>

DirectX::Mouse::Mode BasicTriangleExample::getInitialMouseMode() {
    return DirectX::Mouse::MODE_ABSOLUTE;
}

HRESULT BasicTriangleExample::setup() {
    // Create layout which corresponds to the vertices we use - see VertexTypes::PosVertex in VertexTypes.h
    std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    // Create a simple triangle to draw in render
    triangle_ = std::make_unique<Triangle>(context_.d3dDevice_);
    // Specify and create shader from given fx files and entrypoints and layout. Make sure that layout corresponds to vertex shader input.
    shader_ = std::make_unique<SimpleShader>(context_.d3dDevice_, "shaders/BasicTriangle.fx", "VS", "shaders/BasicTriangle.fx", "PS", layout);

    return S_OK;
}

void BasicTriangleExample::render() {
    // Clear the backbuffer with blue color
    context_.immediateContext_->ClearRenderTargetView(context_.renderTargetView_, Util::srgbToLinear(DirectX::Colors::MidnightBlue));
    // Clear the depth and stencil buffer
    context_.immediateContext_->ClearDepthStencilView(context_.depthStencilView_, D3D11_CLEAR_DEPTH, 1.0f, 0);

    // Use the simple shader
    shader_->use(context_.immediateContext_);
    // Draw the triangle to backbuffer
    triangle_->draw(context_);

    // Put backbuffer in front
    context_.swapChain_->Present(0, 0);
}
