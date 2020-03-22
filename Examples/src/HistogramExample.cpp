#include "HistogramExample.h"

#include "Transform.h"

#include "DDSTextureLoader.h"

#include <directxcolors.h>

using namespace DirectX;

namespace Compute {

constexpr int HISTOGRAM_LEVELS  = 256; // color levels

int HIST_DISPL_WIDTH = 512;
int HIST_DISPL_HEIGHT = 256;

HRESULT HistogramExample::setup() {
    auto hr = BaseExample::setup();

    #if 0
    D3D11_TEXTURE1D_DESC texDesc{};
    texDesc.Width           = 256; // For 256 color levels
    texDesc.MipLevels       = 1;
    texDesc.ArraySize       = 0;
    texDesc.Format          = DXGI_FORMAT_R10G10B10A2_TYPELESS;
    texDesc.Usage           = D3D11_USAGE_DEFAULT; // Access will be only from shaders, read and write
    texDesc.BindFlags       = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags  = 0;
    texDesc.MiscFlags       = 0;
    
    ComPtr<ID3D11Texture1D> histogramTex;
    hr = context_.d3dDevice_->CreateTexture1D(&texDesc, nullptr, histogramTex.GetAddressOf());
    if (FAILED(hr))
        return hr;

    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
    uavDesc.Format              = DXGI_FORMAT_R10G10B10A2_UNORM;
    uavDesc.ViewDimension       = D3D11_UAV_DIMENSION_TEXTURE1D;
    uavDesc.Texture1D.MipSlice  = 0;

    hr = context_.d3dDevice_->CreateUnorderedAccessView(histogramTex.Get(), &uavDesc, histDataUAV_.GetAddressOf());
    if (FAILED(hr))
        return hr;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format                      = DXGI_FORMAT_R10G10B10A2_UNORM;
    srvDesc.ViewDimension               = D3D11_SRV_DIMENSION_TEXTURE1D;
    srvDesc.Texture1D.MipLevels         = 1;
    srvDesc.Texture1D.MostDetailedMip   = 0;
    #endif

    pointSampler_ = std::make_unique<PointWrapSampler>(context_.d3dDevice_);
    quad_ = std::make_unique<Quad>(context_.d3dDevice_);

    hr = CreateDDSTextureFromFile(context_.d3dDevice_, L"textures/seafloor.dds", true, reinterpret_cast<ID3D11Resource**>(srcTexture_.GetAddressOf()), srcTextureSRV_.GetAddressOf());
    if (FAILED(hr))
        return hr;

    histDataCB_ = std::make_unique<ConstBuffHistData>(context_.d3dDevice_);
    histDisplCB_ = std::make_unique<ConstBuffHistDispl>(context_.d3dDevice_);

    for (int i = 0; i < FRAMES_IN_FLIGHT; ++i)
    {
        constexpr int HISTOGRAM_BPP     = 16; // bytes per pixel
        D3D11_BUFFER_DESC buffDesc{};
        buffDesc.ByteWidth              = HISTOGRAM_LEVELS * HISTOGRAM_BPP; // For 256 color levels
        buffDesc.Usage                  = D3D11_USAGE_DEFAULT; // Access will be only from shaders, read and write
        buffDesc.BindFlags              = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
        buffDesc.CPUAccessFlags         = 0;
        buffDesc.MiscFlags              = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        buffDesc.StructureByteStride    = HISTOGRAM_BPP;
    
        ComPtr<ID3D11Buffer> histogramBuff;
        hr = context_.d3dDevice_->CreateBuffer(&buffDesc, nullptr, histogramBuff.GetAddressOf());
        if (FAILED(hr))
            return hr;

        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
        uavDesc.Format              = DXGI_FORMAT_UNKNOWN;
        uavDesc.ViewDimension       = D3D11_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.FirstElement = 0;
        uavDesc.Buffer.NumElements  = HISTOGRAM_LEVELS;
        uavDesc.Buffer.Flags        = 0;

        hr = context_.d3dDevice_->CreateUnorderedAccessView(histogramBuff.Get(), &uavDesc, histDataUAV_[i].GetAddressOf());
        if (FAILED(hr))
            return hr;

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Format                      = DXGI_FORMAT_UNKNOWN;
        srvDesc.ViewDimension               = D3D11_SRV_DIMENSION_BUFFER;
        srvDesc.Buffer.FirstElement         = 0;
        srvDesc.Buffer.NumElements          = HISTOGRAM_LEVELS;

        hr = context_.d3dDevice_->CreateShaderResourceView(histogramBuff.Get(), &srvDesc, histDataSRV_[i].GetAddressOf());
        if (FAILED(hr))
            return hr;
    }

    for (int i = 0; i < FRAMES_IN_FLIGHT; ++i)
    {
        D3D11_TEXTURE2D_DESC histTexDesc{};
        histTexDesc.Width          = 512;
        histTexDesc.Height         = 256;
        histTexDesc.MipLevels      = 1;
        histTexDesc.ArraySize      = 1;
        histTexDesc.Format         = DXGI_FORMAT_R8G8B8A8_TYPELESS;
        histTexDesc.SampleDesc     = { 1, 0 };
        histTexDesc.Usage          = D3D11_USAGE_DEFAULT;
        histTexDesc.BindFlags      = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
        histTexDesc.CPUAccessFlags = 0;
        histTexDesc.MiscFlags      = 0;

        ComPtr<ID3D11Texture2D> histogramTexture;
        hr = context_.d3dDevice_->CreateTexture2D(&histTexDesc, nullptr, histogramTexture.GetAddressOf());
        if (FAILED(hr))
            return hr;

        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
        uavDesc.Format              = DXGI_FORMAT_R8G8B8A8_UNORM;
        uavDesc.ViewDimension       = D3D11_UAV_DIMENSION_TEXTURE2D;
        uavDesc.Texture2D.MipSlice  = 0;

        hr = context_.d3dDevice_->CreateUnorderedAccessView(histogramTexture.Get(), &uavDesc, histDisplUAV_[i].GetAddressOf());
        if (FAILED(hr))
            return hr;

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Format                      = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        srvDesc.ViewDimension               = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels         = 1;
        srvDesc.Texture2D.MostDetailedMip   = 0;

        hr = context_.d3dDevice_->CreateShaderResourceView(histogramTexture.Get(), &srvDesc, histDisplSRV_[i].GetAddressOf());
    }

    hr = reloadShaders();
    if (FAILED(hr))
        return hr;

    return S_OK;
}

bool HistogramExample::reloadShadersInternal() {
    {
        auto shader = std::make_unique<ComputeShader>(context_.d3dDevice_, "shaders/HistDataCS.fx", "main");
        if (!Shaders::isLastCompileOK) {
            return false;
        }
        histDataCS_ = std::move(shader);
    }

    {
        auto shader = std::make_unique<ComputeShader>(context_.d3dDevice_, "shaders/HistDisplCS.fx", "main");
        if (!Shaders::isLastCompileOK) {
            return false;
        }
        histDisplCS_ = std::move(shader);
    }

    return Shaders::makeShader<Shaders::TexturedQuad>(histDisplayShader_, context_.d3dDevice_, "shaders/TexturedQuad.fx", "VS", "shaders/TexturedQuad.fx", "PS", Layouts::POS_UV_LAYOUT);
}

void HistogramExample::handleInput() {
    BaseExample::handleInput();
}

void HistogramExample::render() {
    BaseExample::render();

    int frameIdx = frameCount_ % FRAMES_IN_FLIGHT;

    clearViews();
    // Render scene

    D3D11_TEXTURE2D_DESC srcTexDesc;
    srcTexture_->GetDesc(&srcTexDesc);
    const int texWidth = srcTexDesc.Width;
    const int texHeight = srcTexDesc.Height;

    const int totalPixels = texWidth * texHeight;
    const float histPerPixel = 1.0f / totalPixels;

    
    // Compute the histogram of given image (scene)
    HistDataCB constBuff{};
    constBuff.Dimensions.x = texWidth;
    constBuff.Dimensions.y = texHeight;
    constBuff.Dimensions.z = HISTOGRAM_LEVELS;
    constBuff.HistPerPix.x = histPerPixel;
    histDataCB_->update(context_.immediateContext_, constBuff);
    histDataCB_->use<Stage::CS>(context_.immediateContext_, 0);
    histDataCS_->use(context_.immediateContext_);

    UINT clr[4] = { 0, 0, 0, 0 };
    context_.immediateContext_->ClearUnorderedAccessViewUint(histDataUAV_[frameIdx].Get(), clr);

    // This must match with the values in the shader
    static constexpr int THREAD_PER_GROUP_X = 8;
    static constexpr int THREAD_PER_GROUP_Y = 8;
    static constexpr int THREAD_PER_GROUP_Z = 1;
    const int threadGroupCountX = static_cast<int>(std::ceilf(1.0f * texWidth / THREAD_PER_GROUP_X));
    const int threadGroupCountY = static_cast<int>(std::ceilf(1.0f * texHeight / THREAD_PER_GROUP_Y));

    context_.immediateContext_->CSSetShaderResources(0, 1, srcTextureSRV_.GetAddressOf());
    context_.immediateContext_->CSSetUnorderedAccessViews(0, 1, histDataUAV_[frameIdx].GetAddressOf(), nullptr);
    context_.immediateContext_->Dispatch(threadGroupCountX, threadGroupCountY, 1);

    static ID3D11ShaderResourceView* nullSRV = { nullptr };
    static ID3D11UnorderedAccessView* nullUAV = { nullptr };
    context_.immediateContext_->CSSetShaderResources(0, 1, &nullSRV);
    context_.immediateContext_->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);


    // Render histogram display to texture
    HistDisplCB constBuffDispl{};
    constBuffDispl.HistIdx_NumPix.x = 0;
    constBuffDispl.HistIdx_NumPix.y = totalPixels;
    histDisplCB_->update(context_.immediateContext_, constBuffDispl);
    histDisplCB_->use<Stage::CS>(context_.immediateContext_, 0);
    histDisplCS_->use(context_.immediateContext_);

    context_.immediateContext_->CSSetShaderResources(0, 1, histDataSRV_[frameIdx].GetAddressOf());
    context_.immediateContext_->CSSetUnorderedAccessViews(0, 1, histDisplUAV_[frameIdx].GetAddressOf(), nullptr);
    context_.immediateContext_->Dispatch(HISTOGRAM_LEVELS, 1, 1);

    context_.immediateContext_->CSSetShaderResources(0, 1, &nullSRV);
    context_.immediateContext_->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);


    // Render histogram texture to screen
    const float scale = 0.4f;
    Transform shadowMapDisplayTransform(
        XMFLOAT3(0, 0, 0),
        XMFLOAT3(0, 0, 0),
        XMFLOAT3(scale, scale, scale)
    );
    Shaders::TexturedQuadCB texQuadCB;
    texQuadCB.World = XMMatrixTranspose(shadowMapDisplayTransform.generateModelMatrix());

    histDisplayShader_->use(context_.immediateContext_);
    histDisplayShader_->updateConstantBuffer(context_.immediateContext_, texQuadCB);
    context_.immediateContext_->PSSetShaderResources(0, 1, histDisplSRV_[frameIdx].GetAddressOf());
    pointSampler_->use(context_.immediateContext_, 0);
    quad_->draw(context_);

    context_.immediateContext_->PSSetShaderResources(0, 1, &nullSRV);

    context_.swapChain_->Present(1, 0);

}

}

