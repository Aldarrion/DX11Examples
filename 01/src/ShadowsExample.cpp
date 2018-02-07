#include "ShadowsExample.h"
#include "DDSTextureLoader.h"
#include <directxcolors.h>
#include <minwinbase.h>
#include <minwinbase.h>

using namespace DirectX;

namespace Shadows {
HRESULT ShadowsExample::setup() {
    BasicExample::setup();

    std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    shadowShader_ = std::make_unique<ShadowShader>(context_.d3dDevice_, L"shaders/Shadows.fx", "VS_Shadow", L"shaders/Shadows.fx", "PS_Shadow", layout);
    texturedPhong_ = std::make_unique<TextureShader>(context_.d3dDevice_, L"shaders/PhongShadows.fx", "VS", L"shaders/PhongShadows.fx", "PS", layout);
    texturedCube_ = std::make_unique<TexturedCube>(context_.d3dDevice_);

    auto hr = CreateDDSTextureFromFile(context_.d3dDevice_, L"textures/seafloor.dds", nullptr, &seaFloorTexture_);
    if (FAILED(hr))
        return hr;

    shadowSampler_ = std::make_unique<ShadowSampler>(context_.d3dDevice_);
    linearSampler_ = std::make_unique<LinearSampler>(context_.d3dDevice_);

    std::vector<D3D11_INPUT_ELEMENT_DESC> layoutSolid = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    solidShader_ = std::make_unique<SolidShader>(context_.d3dDevice_, L"shaders/Solid.fx", "VS", L"shaders/Solid.fx", "PSSolid", layoutSolid);
    colorCube_ = std::make_unique<ColorCube>(context_.d3dDevice_);

    // =======
    // Shadows
    // =======
    D3D11_TEXTURE2D_DESC texDesc;
    ZeroMemory(&texDesc, sizeof(D3D11_TEXTURE2D_DESC));
    texDesc.Width = context_.WIDTH;
    texDesc.Height = context_.HEIGHT;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;

    hr = context_.d3dDevice_->CreateTexture2D(&texDesc, nullptr, &shadowMap_);
    if (FAILED(hr)) 
        return hr;

    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory(&descDSV, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
    descDSV.Format = DXGI_FORMAT_D32_FLOAT;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;

    hr = context_.d3dDevice_->CreateDepthStencilView(shadowMap_, &descDSV, &shadowMapDepthView_);
    if (FAILED(hr)) 
        return hr;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;

    hr = context_.d3dDevice_->CreateShaderResourceView(shadowMap_, &srvDesc, &shadowShaderResourceView_);
    if (FAILED(hr)) 
        return hr;

    return S_OK;
}

void ShadowsExample::renderScene(const XMMATRIX& projection, const XMMATRIX& viewMatrix, const XMFLOAT3& viewPos) const {
    
}

void ShadowsExample::render() {
    BasicExample::render();

    const XMFLOAT4 sunPos = XMFLOAT4(-30.0f, 30.0f, -30.0f, 1.0f);
    const XMMATRIX lightProjection = XMMatrixOrthographicLH(20.0f, 20.0f, 1.0f, 100.0f);
    const auto focus = XMFLOAT3(0, 0, 0);
    const auto up = XMFLOAT3(0, 1, 0);
    const XMMATRIX lightView = XMMatrixLookAtLH(XMLoadFloat4(&sunPos), XMLoadFloat3(&focus), XMLoadFloat3(&up));
    const XMFLOAT4 planePos = XMFLOAT4(0.0, -4.0f, 0.0f, 1.0f);
    const XMMATRIX planeScale = XMMatrixScaling(20.0f, 2.2f, 20.0f);

    {
        // Draw shadows
        context_.immediateContext_->OMSetRenderTargets(0, nullptr, shadowMapDepthView_);
        context_.immediateContext_->ClearDepthStencilView(shadowMapDepthView_, D3D11_CLEAR_DEPTH, 1.0f, 0);

        // Draw cube
        ShadowConstBuffer cb;
        cb.World = XMMatrixIdentity();
        cb.Projection = XMMatrixTranspose(lightProjection);
        cb.View = XMMatrixTranspose(lightView);

        shadowShader_->updateConstantBuffer(context_.immediateContext_, cb);
        shadowShader_->use(context_.immediateContext_);
        texturedCube_->draw(context_.immediateContext_);

        // Draw floor
        cb.World = XMMatrixTranspose(planeScale * XMMatrixTranslationFromVector(XMLoadFloat4(&planePos)));

        shadowShader_->updateConstantBuffer(context_.immediateContext_, cb);
        texturedCube_->draw(context_.immediateContext_);
    }

    // Draw rest
    {
        context_.immediateContext_->OMSetRenderTargets(1, &context_.renderTargetView_, context_.depthStencilView_);
        context_.immediateContext_->ClearRenderTargetView(context_.renderTargetView_, Colors::MidnightBlue);
        context_.immediateContext_->ClearDepthStencilView(context_.depthStencilView_, D3D11_CLEAR_DEPTH, 1.0f, 0);

        // Draw cube
        ConstantBuffer cb;
        cb.World = XMMatrixIdentity();
        cb.NormalMatrix = computeNormalMatrix(cb.World);
        //cb.Projection = XMMatrixTranspose(lightProjection);
        cb.Projection = XMMatrixTranspose(projection_);
        //cb.View = XMMatrixTranspose(lightView);
        cb.View = XMMatrixTranspose(camera_.GetViewMatrix());
        cb.LightView = XMMatrixTranspose(lightView);
        cb.LightProjection = XMMatrixTranspose(lightProjection);
        cb.ViewPos = camera_.Position;
        cb.DirLightCount = 1;
        cb.DirLights[0].Color = SUN_YELLOW;
        cb.DirLights[0].Direction = XMFLOAT4(-sunPos.x, -sunPos.y, -sunPos.z, 1.0f);

        texturedPhong_->updateConstantBuffer(context_.immediateContext_, cb);
        texturedPhong_->use(context_.immediateContext_);
        context_.immediateContext_->PSSetShaderResources(0, 1, &seaFloorTexture_);
        context_.immediateContext_->PSSetShaderResources(1, 1, &shadowShaderResourceView_);
        linearSampler_->use(context_.immediateContext_, 0);
        shadowSampler_->use(context_.immediateContext_, 1);
        texturedCube_->draw(context_.immediateContext_);

        // Draw floor
        cb.World = XMMatrixTranspose(planeScale * XMMatrixTranslationFromVector(XMLoadFloat4(&planePos)));
        cb.NormalMatrix = computeNormalMatrix(cb.World);

        texturedPhong_->updateConstantBuffer(context_.immediateContext_, cb);
        texturedCube_->draw(context_.immediateContext_);

        // Draw sun
        /*SolidConstBuffer scb;
        scb.OutputColor = SUN_YELLOW;
        scb.Projection = XMMatrixTranspose(projection_);
        scb.View = XMMatrixTranspose(camera_.GetViewMatrix());
        const XMMATRIX scale = XMMatrixScaling(0.2f, 0.2f, 0.2f);
        scb.World = XMMatrixTranspose(scale * XMMatrixTranslationFromVector(XMLoadFloat4(&sunPos)));

        solidShader_->updateConstantBuffer(context_.immediateContext_, scb);
        solidShader_->use(context_.immediateContext_);
        colorCube_->draw(context_.immediateContext_);*/

        context_.swapChain_->Present(0, 0);
    }
}
}
