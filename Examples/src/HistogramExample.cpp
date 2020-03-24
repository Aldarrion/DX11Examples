#include "HistogramExample.h"
#include "Transform.h"
#include "DDSTextureLoader.h"

#include "stb/stb_image.h"

#include <directxcolors.h>

using namespace DirectX;

namespace Compute {

constexpr int HISTOGRAM_LEVELS  = 256; // color levels + 1 to store max values

int HIST_DISPL_WIDTH = 512;
int HIST_DISPL_HEIGHT = 256;

std::string to_string(const HistogramExample::HistogramMode mode) {
    switch (mode) {
        case HistogramExample::HM_RED: return "Red";
        case HistogramExample::HM_GREEN: return "Green";
        case HistogramExample::HM_BLUE: return "Blue";
        case HistogramExample::HM_ALL: return "All";
        default: return "INVALID_VALUE";
    }
}

void HistogramExample::updateText() {
    using std::to_string;
    infoText_->setText(
        "\n " + to_string(prevHistMode_) + ": previous histogram mode"
        + "\n " + to_string(nextHistMode_) + ": next histogram mode"
        + "\n " "Current mode: " + to_string(HIST_MODES[histModeIdx_])
        + "\n The modes are magnified for beter demonstraion (not comparable)"
    );
}

DirectX::Mouse::Mode HistogramExample::getInitialMouseMode() {
    return Mouse::MODE_ABSOLUTE;
}

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

    Text::makeDefaultSDFFont(context_, font_);
    infoText_ = std::make_unique<Text::TextSDF>("", &font_);
    infoText_->setColor(XMFLOAT4(0, 0, 0, 1));
    updateText();

    pointSampler_ = std::make_unique<PointWrapSampler>(context_.d3dDevice_);
    linearSampler_ = std::make_unique<LinearSampler>(context_.d3dDevice_);
    quad_ = std::make_unique<Quad>(context_.d3dDevice_);


    // ===============================================
    // Load source texture to compute the histogram of
    // ===============================================
    {
        int x, y, n;
        unsigned char *data = stbi_load("textures/clouds-daylight-forest-grass-371589.jpg", &x, &y, &n, 4);

        if (!data)
            return S_FALSE;

        D3D11_SUBRESOURCE_DATA subResourceData{};
        subResourceData.pSysMem = data;
        subResourceData.SysMemPitch = x * 4;

        D3D11_TEXTURE2D_DESC texDesc{};
        texDesc.Width = x;
        texDesc.Height = y;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        texDesc.SampleDesc.Count = 1;
        texDesc.SampleDesc.Quality = 0;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        texDesc.CPUAccessFlags = 0;
        texDesc.MiscFlags = 0;

        hr = context_.d3dDevice_->CreateTexture2D(&texDesc, &subResourceData, srcTexture_.GetAddressOf());
        if (FAILED(hr)) {
            stbi_image_free(data);
            return hr;
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
        srvDesc.Texture2D.MostDetailedMip = 0;

        hr = context_.d3dDevice_->CreateShaderResourceView(srcTexture_.Get(), &srvDesc, srcTextureSRV_.GetAddressOf());
        if (FAILED(hr)) {
            stbi_image_free(data);
            return hr;
        }

        stbi_image_free(data);
    }


    // ==========================================
    // Create the histogram computation resources
    // ==========================================
    histDataCB_ = std::make_unique<ConstBuffHistData>(context_.d3dDevice_);
    histDisplCB_ = std::make_unique<ConstBuffHistDispl>(context_.d3dDevice_);

    {
        constexpr int HISTOGRAM_BPP     = 16; // bytes per pixel
        D3D11_BUFFER_DESC buffDesc{};
        buffDesc.ByteWidth              = HISTOGRAM_LEVELS * HISTOGRAM_BPP; // For color levels
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

        hr = context_.d3dDevice_->CreateUnorderedAccessView(histogramBuff.Get(), &uavDesc, histDataUAV_.GetAddressOf());
        if (FAILED(hr))
            return hr;

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Format                      = DXGI_FORMAT_UNKNOWN;
        srvDesc.ViewDimension               = D3D11_SRV_DIMENSION_BUFFER;
        srvDesc.Buffer.FirstElement         = 0;
        srvDesc.Buffer.NumElements          = HISTOGRAM_LEVELS;

        hr = context_.d3dDevice_->CreateShaderResourceView(histogramBuff.Get(), &srvDesc, histDataSRV_.GetAddressOf());
        if (FAILED(hr))
            return hr;
    }

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

        hr = context_.d3dDevice_->CreateUnorderedAccessView(histogramTexture.Get(), &uavDesc, histDisplUAV_.GetAddressOf());
        if (FAILED(hr))
            return hr;

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Format                      = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        srvDesc.ViewDimension               = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels         = 1;
        srvDesc.Texture2D.MostDetailedMip   = 0;

        hr = context_.d3dDevice_->CreateShaderResourceView(histogramTexture.Get(), &srvDesc, histDisplSRV_.GetAddressOf());
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

    return Shaders::makeShader<Shaders::TexturedQuad>(texturedQuadShader_, context_.d3dDevice_, "shaders/TexturedQuad.fx", "VS", "shaders/TexturedQuad.fx", "PS", Layouts::POS_UV_LAYOUT);
}

void HistogramExample::handleInput() {
    BaseExample::handleInput();

    if (GetAsyncKeyState(prevHistMode_) & 1) {
        histModeIdx_ = histModeIdx_ == 0 ? HIST_MODE_COUNT - 1 : histModeIdx_ - 1;
        updateText();
    }
    if (GetAsyncKeyState(nextHistMode_) & 1) {
        histModeIdx_ = histModeIdx_ == (HIST_MODE_COUNT - 1) ? 0 : histModeIdx_ + 1;
        updateText();
    }
}

void HistogramExample::render() {
    BaseExample::render();

    clearViews();
    
    // Get the current texture data
    D3D11_TEXTURE2D_DESC srcTexDesc;
    srcTexture_->GetDesc(&srcTexDesc);
    const int texWidth = srcTexDesc.Width;
    const int texHeight = srcTexDesc.Height;

    // Compute the histogram of given image (scene)
    HistDataCB constBuff{};
    constBuff.Dimensions.x = texWidth;
    constBuff.Dimensions.y = texHeight;
    constBuff.Dimensions.z = HISTOGRAM_LEVELS;
    histDataCB_->update(context_.immediateContext_, constBuff);
    histDataCB_->use<Stage::CS>(context_.immediateContext_, 0);
    histDataCS_->use(context_.immediateContext_);

    // Clear the UAV buffer used to accumulate the histogram data
    UINT clr[4] = { 0, 0, 0, 0 };
    context_.immediateContext_->ClearUnorderedAccessViewUint(histDataUAV_.Get(), clr);

    // This must match with the values in the shader
    static constexpr int THREAD_PER_GROUP_X = 16;
    static constexpr int THREAD_PER_GROUP_Y = 16;
    static constexpr int THREAD_PER_GROUP_Z = 1;
    const int threadGroupCountX = static_cast<int>(std::ceilf(1.0f * texWidth / THREAD_PER_GROUP_X));
    const int threadGroupCountY = static_cast<int>(std::ceilf(1.0f * texHeight / THREAD_PER_GROUP_Y));

    context_.immediateContext_->CSSetShaderResources(0, 1, srcTextureSRV_.GetAddressOf());
    context_.immediateContext_->CSSetUnorderedAccessViews(0, 1, histDataUAV_.GetAddressOf(), nullptr);

    // Dispatch is an execution command for compute shaders, which spawns a number of compute groups given in its arguments
    // The number of threads in these groups is controlled from the compute shader (see numthreads[x, y, z])
    context_.immediateContext_->Dispatch(threadGroupCountX, threadGroupCountY, 1);

    // UAV needs to be unbound before it is used on input, otherwise it is forced to null by the driver
    static ID3D11UnorderedAccessView* nullUAV = { nullptr };
    context_.immediateContext_->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);


    // Render histogram display to texture
    HistDisplCB constBuffDispl{};
    constBuffDispl.HistIdx.x = HIST_MODES[histModeIdx_];
    histDisplCB_->update(context_.immediateContext_, constBuffDispl);
    histDisplCB_->use<Stage::CS>(context_.immediateContext_, 0);
    histDisplCS_->use(context_.immediateContext_);

    context_.immediateContext_->CSSetShaderResources(0, 1, histDataSRV_.GetAddressOf());
    context_.immediateContext_->CSSetUnorderedAccessViews(0, 1, histDisplUAV_.GetAddressOf(), nullptr);
    context_.immediateContext_->Dispatch(HISTOGRAM_LEVELS, 1, 1);

    context_.immediateContext_->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);


    // Render the source texture to screen
    {
        Transform shadowMapDisplayTransform(XMFLOAT3(0, 0, 0));
        Shaders::TexturedQuadCB texQuadCB;
        texQuadCB.World = XMMatrixTranspose(shadowMapDisplayTransform.generateModelMatrix());

        texturedQuadShader_->use(context_.immediateContext_);
        texturedQuadShader_->updateConstantBuffer(context_.immediateContext_, texQuadCB);
        context_.immediateContext_->PSSetShaderResources(0, 1, srcTextureSRV_.GetAddressOf());
        linearSampler_->use(context_.immediateContext_, 0);
        quad_->draw(context_);
    }


    // Render histogram texture to screen
    {
        const float scale = 0.4f;
        Transform shadowMapDisplayTransform(
            XMFLOAT3(1 - scale, 1 - scale, 0),
            XMFLOAT3(0, 0, 0),
            XMFLOAT3(scale, scale, scale)
        );
        Shaders::TexturedQuadCB texQuadCB;
        texQuadCB.World = XMMatrixTranspose(shadowMapDisplayTransform.generateModelMatrix());

        texturedQuadShader_->use(context_.immediateContext_);
        texturedQuadShader_->updateConstantBuffer(context_.immediateContext_, texQuadCB);
        context_.immediateContext_->PSSetShaderResources(0, 1, histDisplSRV_.GetAddressOf());
        pointSampler_->use(context_.immediateContext_, 0);
        quad_->draw(context_);
    }

    //context_.immediateContext_->PSSetShaderResources(0, 1, &nullSRV);

    infoText_->draw(context_);

    context_.swapChain_->Present(1, 0);
}

}

