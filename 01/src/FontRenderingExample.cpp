#include "FontRenderingExample.h"

namespace FontRendering {

std::string specString(float spec) {
    return "To adjust specularity press K/L: " + std::to_string(spec);
}

HRESULT FontRenderingExample::setup() {
    BaseExample::setup();

    text_ = std::make_unique<Text::Text>(context_.d3dDevice_, context_.immediateContext_, "To adjust specularity press K/L: 0");

    // TODO move alpha blending to context wrapper

    // =====================
    // Enable alpha blending
    // =====================

    D3D11_BLEND_DESC blendDesc;
    ZeroMemory(&blendDesc, sizeof D3D11_BLEND_DESC);
    blendDesc.RenderTarget[0].BlendEnable = true;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    ID3D11BlendState* blendState;
    auto hr = context_.d3dDevice_->CreateBlendState(&blendDesc, &blendState);
    if (FAILED(hr)) {
        MessageBox(nullptr, L"Failed to create blend state", L"Error", MB_OK);
        return hr;
    }

    float bl[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    context_.immediateContext_->OMSetBlendState(blendState, bl, 0xffffffff);

    return S_OK;
}

void FontRenderingExample::render() {
    BaseExample::render();

    text_->setText(specString(deltaTime_));

    clearViews();

    text_->draw(context_.immediateContext_, context_.WIDTH / static_cast<float>(context_.HEIGHT));

    context_.swapChain_->Present(0, 0);
}
}
