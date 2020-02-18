#include "AlphaToCoverageExample.h"

#include "WinKeyMap.h"

#include <directxcolors.h>


using namespace DirectX;

namespace {
std::string getNameOfMode(AlphaToCoverage::BlendMode mode) {
    switch (mode)
    {
        case AlphaToCoverage::AlphaToCoverage:
            return "Alpha to coverage";
        case AlphaToCoverage::Blend:
            return "Alpha blending";
        case AlphaToCoverage::NoBlend:
            return "No blending";
        default:
            return "Unknown";
    }
}

std::string getHelperText(AlphaToCoverage::BlendMode mode) {
    return "\nPress E to cycle modes\nMode: " + getNameOfMode(mode);
}

}

namespace AlphaToCoverage {

HRESULT AlphaToCoverageExample::setup() {
    auto hr = BaseExample::setup();
    if (FAILED(hr))
        return hr;

    texture_ = std::make_unique<Texture>(context_.d3dDevice_, context_.immediateContext_, L"textures/AlphaToCoverage.dds", true);
    sampler_ = std::make_unique<AnisotropicSampler>(context_.d3dDevice_);

    quad_ = std::make_unique<Quad>(context_.d3dDevice_);

    hr = reloadShaders();
    if (FAILED(hr))
        return hr;

    D3D11_BLEND_DESC blendDesc;
    ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
    blendDesc.AlphaToCoverageEnable = true;
    blendDesc.RenderTarget[0].BlendEnable = false;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    hr = context_.d3dDevice_->CreateBlendState(&blendDesc, &alphaToCoverageBlendState_);
    if (FAILED(hr)) {
        MessageBoxA(nullptr, "Failed to create blend state", "Error", MB_OK);
        return hr;
    }

    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.RenderTarget[0].BlendEnable = true;
    hr = context_.d3dDevice_->CreateBlendState(&blendDesc, &alphaBlendingBlendState_);
    if (FAILED(hr)) {
        MessageBoxA(nullptr, "Failed to create blend state", "Error", MB_OK);
        return hr;
    }

    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.RenderTarget[0].BlendEnable = false;
    hr = context_.d3dDevice_->CreateBlendState(&blendDesc, &noBlendingBlendState_);
    if (FAILED(hr)) {
        MessageBoxA(nullptr, "Failed to create blend state", "Error", MB_OK);
        return hr;
    }


    float bl[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    context_.immediateContext_->OMSetBlendState(alphaToCoverageBlendState_, bl, 0xffffffff);

    text_ = std::make_unique<Text::Text>(context_.d3dDevice_, context_.immediateContext_, getHelperText(blendMode_));
    text_->setColor(DirectX::XMFLOAT4(0, 0, 0, 1));

    return hr;
}

bool AlphaToCoverageExample::reloadShadersInternal() {
    return Shaders::makeShader<AtoCShader>(
        shader_,
        context_.d3dDevice_,
        L"shaders/AlphaToCoverage.fx", "VS",
        L"shaders/AlphaToCoverage.fx", "PS",
        quad_->getVertexLayout()
    );
}

void AlphaToCoverageExample::handleInput() {
    BaseExample::handleInput();
    if (GetAsyncKeyState(WinKeyMap::E) & 1) {
        blendMode_ = BlendMode((blendMode_ + 1) % COUNT);
        
        ID3D11BlendState* blendState;
        switch (blendMode_)
        {
            case AlphaToCoverage:
                blendState = alphaToCoverageBlendState_;
                break;
            case Blend:
                blendState = alphaBlendingBlendState_;
                break;
            case NoBlend:
                blendState = noBlendingBlendState_;
                break;
            default:
                blendState = alphaToCoverageBlendState_;
                break;
        }

        float bl[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        context_.immediateContext_->OMSetBlendState(blendState, bl, 0xffffffff);

        text_->setText(getHelperText(blendMode_));
    }
}

void AlphaToCoverageExample::render() {
    BaseExample::render();

    context_.immediateContext_->ClearRenderTargetView(context_.renderTargetView_, Util::srgbToLinear(DirectX::Colors::Red));
    context_.immediateContext_->ClearDepthStencilView(context_.depthStencilView_, D3D11_CLEAR_DEPTH, 1.0f, 0);

    text_->draw(context_.immediateContext_, context_.getAspectRatio());

    shader_->use(context_.immediateContext_);
    texture_->use(context_.immediateContext_, 0);
    quad_->draw(context_.immediateContext_);

    context_.swapChain_->Present(0, 0);
}

}
