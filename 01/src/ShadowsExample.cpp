#include "ShadowsExample.h"
#include <directxcolors.h>
#include <minwinbase.h>
#include "Layouts.h"
#include "Transform.h"

using namespace DirectX;

namespace Shadows {
HRESULT ShadowsExample::setup() {
    BasicExample::setup();

    // Shaders
    shadowShader_ = std::make_unique<ShadowShader>(context_.d3dDevice_, L"shaders/Shadows.fx", "VS_Shadow", L"shaders/Shadows.fx", "PS_Shadow", Layouts::TEXTURED_LAYOUT);
    texturedPhong_ = std::make_unique<TextureShader>(context_.d3dDevice_, L"shaders/PhongShadows.fx", "VS", L"shaders/PhongShadows.fx", "PS", Layouts::TEXTURED_LAYOUT);
    solidShader_ = std::make_unique<SolidShader>(context_.d3dDevice_, L"shaders/Solid.fx", "VS", L"shaders/Solid.fx", "PSSolid", Layouts::POS_NORM_COL_LAYOUT);

    // Objects
    texturedCube_ = std::make_unique<TexturedCube>(context_.d3dDevice_);
    colorCube_ = std::make_unique<ColorCube>(context_.d3dDevice_);

    // Textures
    seaFloorTexture_ = std::make_unique<Texture>(context_.d3dDevice_, L"textures/seafloor.dds");

    // Samplers
    linearSampler_ = std::make_unique<LinearSampler>(context_.d3dDevice_);
    // Shadows need point sampler, filtering needs to be done afterwards.
    // Averaging depths would do no good
    shadowSampler_ = std::make_unique<ShadowSampler>(context_.d3dDevice_);
    

    // =======
    // Shadows
    // =======
    // If we want the shadowmap to be different size than our standard viewport
    // we need to create new one and bind it when we generate the shadowmap
    shadowViewPort_.Width = static_cast<FLOAT>(SHADOW_MAP_WIDTH);
    shadowViewPort_.Height = static_cast<FLOAT>(SHADOW_MAP_HEIGHT);
    shadowViewPort_.MinDepth = 0.0f;
    shadowViewPort_.MaxDepth = 1.0f;
    shadowViewPort_.TopLeftX = 0;
    shadowViewPort_.TopLeftY = 0;

    D3D11_TEXTURE2D_DESC texDesc;
    ZeroMemory(&texDesc, sizeof(D3D11_TEXTURE2D_DESC));
    texDesc.Width = SHADOW_MAP_WIDTH;
    texDesc.Height = SHADOW_MAP_HEIGHT;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    // We will look at this texture with 2 different views -> typeless
    texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;

    auto hr = context_.d3dDevice_->CreateTexture2D(&texDesc, nullptr, &shadowMap_);
    if (FAILED(hr)) 
        return hr;

    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory(&descDSV, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
    // Put all precision to the depth
    descDSV.Format = DXGI_FORMAT_D32_FLOAT;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;

    hr = context_.d3dDevice_->CreateDepthStencilView(shadowMap_, &descDSV, &shadowMapDepthView_);
    if (FAILED(hr)) 
        return hr;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    // In shader sample it like regular texture with single red channel
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;

    hr = context_.d3dDevice_->CreateShaderResourceView(shadowMap_, &srvDesc, &shadowShaderResourceView_);
    if (FAILED(hr)) 
        return hr;

    return S_OK;
}

void ShadowsExample::render() {
    BasicExample::render();

    const XMFLOAT4 sunPos = XMFLOAT4(-30.0f, 30.0f, -30.0f, 1.0f);
    const XMMATRIX lightProjection = XMMatrixOrthographicLH(40.0f, 40.0f, 1.0f, 100.0f);
    const auto focus = XMFLOAT3(0, 0, 0);
    const auto up = XMFLOAT3(0, 1, 0);
    const XMMATRIX lightView = XMMatrixLookAtLH(XMLoadFloat4(&sunPos), XMLoadFloat3(&focus), XMLoadFloat3(&up));
    const Transform planeTransform(XMFLOAT3(0.0, -4.0f, 0.0f), XMFLOAT3(), XMFLOAT3(20.0f, 2.2f, 20.0f));
    const std::vector<Transform> cubes = {
        Transform(),
        Transform(XMFLOAT3(2.5f, 1.2f, 1.8f), XMFLOAT3(XMConvertToRadians(45.0f), XMConvertToRadians(30.0f), 0.0f)),
        Transform(XMFLOAT3(-2.5f, -0.8f, -2.5f))
    };

    // ==================
    // Generate shadowmap
    // ==================
    {
        context_.immediateContext_->OMSetRenderTargets(0, nullptr, shadowMapDepthView_);
        context_.immediateContext_->ClearDepthStencilView(shadowMapDepthView_, D3D11_CLEAR_DEPTH, 1.0f, 0);
        context_.immediateContext_->RSSetViewports(1, &shadowViewPort_);

        // Draw cube
        ShadowConstBuffer cb;
        cb.Projection = XMMatrixTranspose(lightProjection);
        cb.View = XMMatrixTranspose(lightView);

        shadowShader_->use(context_.immediateContext_);
        for (const auto& transform : cubes) {
            cb.World = XMMatrixTranspose(transform.GenerateModelMatrix());
            shadowShader_->updateConstantBuffer(context_.immediateContext_, cb);
            texturedCube_->draw(context_.immediateContext_);
        }
        // Draw floor
        cb.World = XMMatrixTranspose(planeTransform.GenerateModelMatrix());

        shadowShader_->updateConstantBuffer(context_.immediateContext_, cb);
        texturedCube_->draw(context_.immediateContext_);
    }

    // Set to true to see how light sees the scene
    const bool drawFromLightView = false;

    // ==============
    // Draw the scene
    // ==============
    {
        context_.immediateContext_->OMSetRenderTargets(1, &context_.renderTargetView_, context_.depthStencilView_);
        context_.immediateContext_->ClearRenderTargetView(context_.renderTargetView_, Colors::MidnightBlue);
        context_.immediateContext_->ClearDepthStencilView(context_.depthStencilView_, D3D11_CLEAR_DEPTH, 1.0f, 0);
        context_.immediateContext_->RSSetViewports(1, &context_.viewPort_);

        // Draw cube
        ConstantBuffer cb;
        if (drawFromLightView) {
            cb.Projection = XMMatrixTranspose(lightProjection);
            cb.View = XMMatrixTranspose(lightView);
        } else {
            cb.Projection = XMMatrixTranspose(projection_);
            cb.View = XMMatrixTranspose(camera_.GetViewMatrix());
        }
        
        cb.LightView = XMMatrixTranspose(lightView);
        cb.LightProjection = XMMatrixTranspose(lightProjection);
        cb.ViewPos = camera_.Position;
        cb.DirLightCount = 1;
        cb.DirLights[0].Color = SUN_YELLOW;
        cb.DirLights[0].Direction = XMFLOAT4(-sunPos.x, -sunPos.y, -sunPos.z, 1.0f);

        texturedPhong_->use(context_.immediateContext_);
        seaFloorTexture_->use(context_.immediateContext_, 0);
        context_.immediateContext_->PSSetShaderResources(1, 1, &shadowShaderResourceView_);
        linearSampler_->use(context_.immediateContext_, 0);
        shadowSampler_->use(context_.immediateContext_, 1);

        for (const auto& transform : cubes) {
            cb.World = XMMatrixIdentity();
            cb.NormalMatrix = computeNormalMatrix(cb.World);
            cb.World = XMMatrixTranspose(transform.GenerateModelMatrix());
            texturedPhong_->updateConstantBuffer(context_.immediateContext_, cb);
            texturedCube_->draw(context_.immediateContext_);
        }

        // Draw floor
        cb.World = XMMatrixTranspose(planeTransform.GenerateModelMatrix());
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
}
