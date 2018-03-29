#include "DeferredRenderingExample.h"

namespace Deferred {

using namespace DirectX;

HRESULT DeferredRenderingExample::setup() {
    BaseExample::setup();

    D3D11_TEXTURE2D_DESC texDesc;
    ZeroMemory(&texDesc, sizeof(D3D11_TEXTURE2D_DESC));
    texDesc.Width = context_.WIDTH;
    texDesc.Height = context_.HEIGHT;
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

    auto hr = context_.d3dDevice_->CreateTexture2D(&texDesc, nullptr, &depthBuffer_);
    if (FAILED(hr))
        return hr;

    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory(&descDSV, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
    // Put all precision to the depth
    descDSV.Format = DXGI_FORMAT_D32_FLOAT;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;

    hr = context_.d3dDevice_->CreateDepthStencilView(depthBuffer_, &descDSV, &depthBufferDepthView_);
    if (FAILED(hr))
        return hr;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    // In shader sample it like regular texture with single red channel
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;

    hr = context_.d3dDevice_->CreateShaderResourceView(depthBuffer_, &srvDesc, &depthBufferResourceView_);
    if (FAILED(hr))
        return hr;


    // ===================
    // Initialize G-buffer
    // ===================
    D3D11_TEXTURE2D_DESC gBufferTextDesc;
    ZeroMemory(&gBufferTextDesc, sizeof(D3D11_TEXTURE2D_DESC));
    gBufferTextDesc.Width = context_.WIDTH;
    gBufferTextDesc.Height = context_.HEIGHT;
    gBufferTextDesc.MipLevels = 1;
    gBufferTextDesc.ArraySize = 1;
    gBufferTextDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    gBufferTextDesc.SampleDesc.Count = 1;
    gBufferTextDesc.SampleDesc.Quality = 0;
    gBufferTextDesc.Usage = D3D11_USAGE_DEFAULT;
    gBufferTextDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    gBufferTextDesc.CPUAccessFlags = 0;
    gBufferTextDesc.MiscFlags = 0;

    hr = context_.d3dDevice_->CreateTexture2D(&gBufferTextDesc, nullptr, &gPosition_);
    if (FAILED(hr))
        return hr;

    hr = context_.d3dDevice_->CreateTexture2D(&gBufferTextDesc, nullptr, &gNormal_);
    if (FAILED(hr))
        return hr;

    hr = context_.d3dDevice_->CreateTexture2D(&gBufferTextDesc, nullptr, &gAlbedo_);
    if (FAILED(hr))
        return hr;

    D3D11_RENDER_TARGET_VIEW_DESC gBufferDesc;
    ZeroMemory(&gBufferDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
    gBufferDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    gBufferDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

    hr = context_.d3dDevice_->CreateRenderTargetView(gPosition_, &gBufferDesc, &gPositionView_);
    if (FAILED(hr))
        return hr;

    hr = context_.d3dDevice_->CreateRenderTargetView(gNormal_, &gBufferDesc, &gNormalView_);
    if (FAILED(hr))
        return hr;

    hr = context_.d3dDevice_->CreateRenderTargetView(gAlbedo_, &gBufferDesc, &gAlbedoView_);
    if (FAILED(hr))
        return hr;


    D3D11_SHADER_RESOURCE_VIEW_DESC gResourceView;
    ZeroMemory(&gResourceView, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
    gResourceView.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    gResourceView.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    gResourceView.Texture2D.MipLevels = texDesc.MipLevels;
    gResourceView.Texture2D.MostDetailedMip = 0;

    hr = context_.d3dDevice_->CreateShaderResourceView(gPosition_, &gResourceView, &gPositionRV_);
    if (FAILED(hr))
        return hr;

    hr = context_.d3dDevice_->CreateShaderResourceView(gNormal_, &gResourceView, &gNormalRV_);
    if (FAILED(hr))
        return hr;

    hr = context_.d3dDevice_->CreateShaderResourceView(gAlbedo_, &gResourceView, &gAlbedoRV_);
    if (FAILED(hr))
        return hr;

    
    // ====================
    // Other initialization
    // ====================
    std::string path = "models/nanosuit/nanosuit.obj";
    model_ = std::make_unique<Models::Model>(context_, path);

    gShader_ = std::make_unique<GShader>(context_.d3dDevice_, L"shaders/DeferredGBuffer.fx", "VS", L"shaders/DeferredGBuffer.fx", "PS", Layouts::POS_NORM_UV_LAYOUT);
    defferedLightShader_ = std::make_unique<DefferedShader>(context_.d3dDevice_, L"shaders/DeferredShader.fx", "VS", L"shaders/DeferredShader.fx", "PS", Layouts::POS_UV_LAYOUT);

    anisoSampler_ = Samplers::createAnisoSampler(context_);
    pointSampler_ = std::make_unique<PointWrapSampler>(context_.d3dDevice_);

    quad_ = std::make_unique<Quad>(context_.d3dDevice_);

    return S_OK;
}

void DeferredRenderingExample::render() {
    BaseExample::render();

    std::array<ID3D11RenderTargetView*, 3> views = {
        gPositionView_,
        gNormalView_,
        gAlbedoView_
    };

    context_.immediateContext_->OMSetRenderTargets(3, views.data(), depthBufferDepthView_);
    for (auto& view : views) {
        context_.immediateContext_->ClearRenderTargetView(view, Colors::MidnightBlue);
    }
    context_.immediateContext_->ClearDepthStencilView(depthBufferDepthView_, D3D11_CLEAR_DEPTH, 1.0f, 0);

    GShaderCB gscb;
    gscb.World = XMMatrixIdentity();
    gscb.NormalMatrix = XMMatrixTranspose(computeNormalMatrix(gscb.World));
    gscb.View = XMMatrixTranspose(camera_.getViewMatrix());
    gscb.Projection = XMMatrixTranspose(projection_);

    gShader_->use(context_.immediateContext_);
    gShader_->updateConstantBuffer(context_.immediateContext_, gscb);
    anisoSampler_->use(context_.immediateContext_, 0);
    model_->draw(context_.immediateContext_);

    // =================
    // Render final quad
    // =================
    DeferredLightCB lightCb;
    lightCb.LightPos = XMFLOAT4(-30.0f, 30.0f, -30.0f, 1.0f);
    lightCb.LightCol = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    context_.immediateContext_->OMSetRenderTargets(1, &context_.renderTargetView_, context_.depthStencilView_);
    context_.immediateContext_->ClearRenderTargetView(context_.renderTargetView_, Colors::MidnightBlue);
    context_.immediateContext_->ClearDepthStencilView(context_.depthStencilView_, D3D11_CLEAR_DEPTH, 1.0f, 0);
    context_.immediateContext_->PSSetShaderResources(0, 1, &gPositionRV_);
    context_.immediateContext_->PSSetShaderResources(1, 1, &gNormalRV_);
    context_.immediateContext_->PSSetShaderResources(2, 1, &gAlbedoRV_);

    defferedLightShader_->updateConstantBuffer(context_.immediateContext_, lightCb);
    defferedLightShader_->use(context_.immediateContext_);
    anisoSampler_->use(context_.immediateContext_, 0);
    pointSampler_->use(context_.immediateContext_, 1);

    quad_->draw(context_.immediateContext_);

    context_.swapChain_->Present(0, 0);
}
}
