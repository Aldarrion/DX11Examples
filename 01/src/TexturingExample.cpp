#include "TexturingExample.h"
#include "DDSTextureLoader.h"
#include <directxcolors.h>

namespace Texturing {

using namespace DirectX;

HRESULT TexturingExample::setup() {
    BasicExample::setup();

    std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    texturedPhong_ = std::make_unique<ShaderProgram<ConstantBuffer>>(context_.d3dDevice_, L"shaders/Textured.fx", "VS", L"shaders/Textured.fx", "PS", layout);
    texturedCube_ = std::make_unique<TexturedCube>(context_.d3dDevice_);

    auto hr = CreateDDSTextureFromFile(context_.d3dDevice_, L"textures/seafloor.dds", nullptr, &seaFloorTexture_);
    if (FAILED(hr))
        return hr;

    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = context_.d3dDevice_->CreateSamplerState(&sampDesc, &textureSampler_);
    if (FAILED(hr))
        return hr;

    std::vector<D3D11_INPUT_ELEMENT_DESC> layoutSolid = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    solidShader_ = std::make_unique<SolidShader>(context_.d3dDevice_, L"shaders/Solid.fx", "VS", L"shaders/Solid.fx", "PSSolid", layoutSolid);
    colorCube_ = std::make_unique<ColorCube>(context_.d3dDevice_);

    return S_OK;
}

void TexturingExample::render() {
    BasicExample::render();

    context_.immediateContext_->ClearRenderTargetView(context_.renderTargetView_, Colors::MidnightBlue);
    context_.immediateContext_->ClearDepthStencilView(context_.depthStencilView_, D3D11_CLEAR_DEPTH, 1.0f, 0);

    // Draw cube
    XMFLOAT4 sunPos = XMFLOAT4(-30.0f, 30.0f, -30.0f, 1.0f);
    
    ConstantBuffer cb;
    cb.World = XMMatrixIdentity();
    cb.NormalMatrix = computeNormalMatrix(cb.World);
    cb.Projection = XMMatrixTranspose(projection_);
    cb.View = XMMatrixTranspose(camera_.GetViewMatrix());
    cb.ViewPos = camera_.Position;
    cb.DirLightCount = 1;
    cb.DirLights[0].Color = SUN_YELLOW;
    cb.DirLights[0].Direction = XMFLOAT4(-sunPos.x, -sunPos.y, -sunPos.z, 1.0f);

    texturedPhong_->updateConstantBuffer(context_.immediateContext_, cb);
    texturedPhong_->use(context_.immediateContext_);
    context_.immediateContext_->PSSetShaderResources(0, 1, &seaFloorTexture_);
    context_.immediateContext_->PSSetSamplers(0, 1, &textureSampler_);
    texturedCube_->draw(context_.immediateContext_);

    // Draw floor
    XMFLOAT4 planePos = XMFLOAT4(0.0, -2.0f, 0.0f, 1.0f);
    const XMMATRIX planeScale = XMMatrixScaling(20.0f, 0.2f, 20.0f);
    cb.World = XMMatrixTranspose(planeScale * XMMatrixTranslationFromVector(XMLoadFloat4(&planePos)));
    cb.NormalMatrix = computeNormalMatrix(cb.World);
    
    texturedPhong_->updateConstantBuffer(context_.immediateContext_, cb);
    texturedCube_->draw(context_.immediateContext_);

    // Draw sun
    SolidConstBuffer scb;
    scb.OutputColor = SUN_YELLOW;
    scb.Projection = XMMatrixTranspose(projection_);
    scb.View = XMMatrixTranspose(camera_.GetViewMatrix());
    const XMMATRIX scale = XMMatrixScaling(0.2f, 0.2f, 0.2f);
    scb.World = XMMatrixTranspose(scale * XMMatrixTranslationFromVector(XMLoadFloat4(&sunPos)));

    solidShader_->updateConstantBuffer(context_.immediateContext_, scb);
    solidShader_->use(context_.immediateContext_);
    colorCube_->draw(context_.immediateContext_);

    context_.swapChain_->Present(0, 0);
}
}
