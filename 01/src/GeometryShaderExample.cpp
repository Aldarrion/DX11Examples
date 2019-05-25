#include "GeometryShaderExample.h"
#include <directxcolors.h>
#include <vector>
#include "Layouts.h"
#include <minwinbase.h>

namespace GeometryShader {

using namespace DirectX;

HRESULT GeometryShaderExample::setup() {
    BaseExample::setup();

    normalShader_ = std::make_unique<GeomShader>(
        context_.d3dDevice_, 
        L"shaders/NormalGeomShader.fx", "VS", 
        L"shaders/NormalGeomShader.fx", "PS", 
        Layouts::TEXTURED_LAYOUT 
        , L"shaders/NormalGeomShader.fx", "GS"
    );

    standardShader_ = std::make_unique<StdShader>(
        context_.d3dDevice_,
        L"shaders/UnlitGeom.fx", "VS",
        L"shaders/UnlitGeom.fx", "PS",
        Layouts::TEXTURED_LAYOUT
    );

    texture_ = std::make_unique<Texture>(context_.d3dDevice_, context_.immediateContext_, L"textures/container2.dds", true);
    sampler_ = std::make_unique<AnisotropicSampler>(context_.d3dDevice_);
    cube_ = std::make_unique<TexturedCube>(context_.d3dDevice_);

    return S_OK;
}

void GeometryShaderExample::render() {
    BaseExample::render();

    context_.immediateContext_->ClearRenderTargetView(context_.renderTargetView_, Util::srgbToLinear(DirectX::Colors::MidnightBlue));
    context_.immediateContext_->ClearDepthStencilView(context_.depthStencilView_, D3D11_CLEAR_DEPTH, 1.0f, 0);

    ConstantBuffer cb;
    cb.World = XMMatrixIdentity();
    cb.View = XMMatrixTranspose(camera_.getViewMatrix());
    cb.Projection = XMMatrixTranspose(projection_);
    cb.NormalMatrix = computeNormalMatrix(cb.World);

    // First draw the scene
    {
        standardShader_->updateConstantBuffer(context_.immediateContext_, cb);
        standardShader_->use(context_.immediateContext_);
        texture_->use(context_.immediateContext_, 0);
        sampler_->use(context_.immediateContext_, 0);
        cube_->draw(context_.immediateContext_);
    }

    // Draw only the normals
    {
        normalShader_->updateConstantBuffer(context_.immediateContext_, cb);
        normalShader_->use(context_.immediateContext_);
        cube_->draw(context_.immediateContext_);
    }

    context_.swapChain_->Present(0, 0);
}
}
