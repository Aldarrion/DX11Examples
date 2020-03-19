#include "SSAOExample.h"
#include "Util.h"

#include <directxcolors.h>

using namespace DirectX;

HRESULT SSAO::SSAOExample::setup() {
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

    ID3D11Texture2D* depthBuffer = nullptr;
    auto hr = context_.d3dDevice_->CreateTexture2D(&texDesc, nullptr, &depthBuffer);
    if (FAILED(hr))
        return hr;

    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory(&descDSV, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
    // Put all precision to the depth
    descDSV.Format = DXGI_FORMAT_D32_FLOAT;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;

    hr = context_.d3dDevice_->CreateDepthStencilView(depthBuffer, &descDSV, depthBufferDepthView_.GetAddressOf());
    if (FAILED(hr))
        return hr;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    // In shader sample it like regular texture with single red channel
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;

    hr = context_.d3dDevice_->CreateShaderResourceView(depthBuffer, &srvDesc, depthBufferResourceView_.GetAddressOf());
    if (FAILED(hr))
        return hr;

    depthBuffer->Release();

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

    ID3D11Texture2D* gPosition = nullptr;
    ID3D11Texture2D* gNormal = nullptr;
    ID3D11Texture2D* gAlbedo = nullptr;

    hr = context_.d3dDevice_->CreateTexture2D(&gBufferTextDesc, nullptr, &gPosition);
    if (FAILED(hr))
        return hr;

    hr = context_.d3dDevice_->CreateTexture2D(&gBufferTextDesc, nullptr, &gNormal);
    if (FAILED(hr))
        return hr;

    hr = context_.d3dDevice_->CreateTexture2D(&gBufferTextDesc, nullptr, &gAlbedo);
    if (FAILED(hr))
        return hr;

    D3D11_RENDER_TARGET_VIEW_DESC gBufferDesc;
    ZeroMemory(&gBufferDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
    gBufferDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    gBufferDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

    hr = context_.d3dDevice_->CreateRenderTargetView(gPosition, &gBufferDesc, gPositionView_.GetAddressOf());
    if (FAILED(hr))
        return hr;

    hr = context_.d3dDevice_->CreateRenderTargetView(gNormal, &gBufferDesc, gNormalView_.GetAddressOf());
    if (FAILED(hr))
        return hr;

    hr = context_.d3dDevice_->CreateRenderTargetView(gAlbedo, &gBufferDesc, gAlbedoView_.GetAddressOf());
    if (FAILED(hr))
        return hr;


    D3D11_SHADER_RESOURCE_VIEW_DESC gResourceView;
    ZeroMemory(&gResourceView, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
    gResourceView.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    gResourceView.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    gResourceView.Texture2D.MipLevels = 1;
    gResourceView.Texture2D.MostDetailedMip = 0;

    hr = context_.d3dDevice_->CreateShaderResourceView(gPosition, &gResourceView, gPositionRV_.GetAddressOf());
    if (FAILED(hr))
        return hr;

    hr = context_.d3dDevice_->CreateShaderResourceView(gNormal, &gResourceView, gNormalRV_.GetAddressOf());
    if (FAILED(hr))
        return hr;

    hr = context_.d3dDevice_->CreateShaderResourceView(gAlbedo, &gResourceView, gAlbedoRV_.GetAddressOf());
    if (FAILED(hr))
        return hr;

    gPosition->Release();
    gNormal->Release();
    gAlbedo->Release();

    // ====
    // SSAO
    // ====
    D3D11_TEXTURE2D_DESC ssaoTexDesc;
    ZeroMemory(&gBufferTextDesc, sizeof(D3D11_TEXTURE2D_DESC));
    ssaoTexDesc.Width = context_.WIDTH;
    ssaoTexDesc.Height = context_.HEIGHT;
    ssaoTexDesc.MipLevels = 1;
    ssaoTexDesc.ArraySize = 1;
    ssaoTexDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    ssaoTexDesc.SampleDesc.Count = 1;
    ssaoTexDesc.SampleDesc.Quality = 0;
    ssaoTexDesc.Usage = D3D11_USAGE_DEFAULT;
    ssaoTexDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    ssaoTexDesc.CPUAccessFlags = 0;
    ssaoTexDesc.MiscFlags = 0;

    ID3D11Texture2D* ssaoBuffer = nullptr;
    ID3D11Texture2D* ssaoBlurBuffer = nullptr;

    hr = context_.d3dDevice_->CreateTexture2D(&ssaoTexDesc, nullptr, &ssaoBuffer);
    if (FAILED(hr))
        return hr;

    hr = context_.d3dDevice_->CreateTexture2D(&ssaoTexDesc, nullptr, &ssaoBlurBuffer);
    if (FAILED(hr))
        return hr;

    D3D11_RENDER_TARGET_VIEW_DESC ssaoRTVDesc;
    ZeroMemory(&ssaoRTVDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
    ssaoRTVDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    ssaoRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

    hr = context_.d3dDevice_->CreateRenderTargetView(ssaoBuffer, &ssaoRTVDesc, ssaoRTView_.GetAddressOf());
    if (FAILED(hr))
        return hr;

    hr = context_.d3dDevice_->CreateRenderTargetView(ssaoBlurBuffer, &ssaoRTVDesc, ssaoBlurRTView_.GetAddressOf());
    if (FAILED(hr))
        return hr;

    D3D11_SHADER_RESOURCE_VIEW_DESC ssaoRVDesc;
    ZeroMemory(&ssaoRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
    ssaoRVDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    ssaoRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    ssaoRVDesc.Texture2D.MipLevels = texDesc.MipLevels;
    ssaoRVDesc.Texture2D.MostDetailedMip = 0;

    hr = context_.d3dDevice_->CreateShaderResourceView(ssaoBuffer, &ssaoRVDesc, ssaoRV_.GetAddressOf());
    if (FAILED(hr))
        return hr;

    hr = context_.d3dDevice_->CreateShaderResourceView(ssaoBlurBuffer, &ssaoRVDesc, ssaoBlurRV_.GetAddressOf());
    if (FAILED(hr))
        return hr;

    ssaoBuffer->Release();
    ssaoBlurBuffer->Release();

    // =============
    // Noise texture
    // =============
    D3D11_TEXTURE2D_DESC noiseBufferDesc;
    ZeroMemory(&noiseBufferDesc, sizeof(D3D11_TEXTURE2D_DESC));
    noiseBufferDesc.Width = 4;
    noiseBufferDesc.Height = 4;
    noiseBufferDesc.MipLevels = 1;
    noiseBufferDesc.ArraySize = 1;
    noiseBufferDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
    noiseBufferDesc.SampleDesc.Count = 1;
    noiseBufferDesc.SampleDesc.Quality = 0;
    noiseBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    noiseBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    noiseBufferDesc.CPUAccessFlags = 0;
    noiseBufferDesc.MiscFlags = 0;

    std::vector<XMFLOAT2>noiseVector = generateNoise();
    D3D11_SUBRESOURCE_DATA noiseData;
    noiseData.pSysMem = static_cast<void*>(noiseVector.data());
    noiseData.SysMemPitch = 4 * sizeof(XMFLOAT2);
    noiseData.SysMemSlicePitch = 0;

    ID3D11Texture2D* noiseBuffer = nullptr;
    hr = context_.d3dDevice_->CreateTexture2D(&noiseBufferDesc, &noiseData, &noiseBuffer);
    if (FAILED(hr))
        return hr;

    D3D11_SHADER_RESOURCE_VIEW_DESC noiseRVDesc;
    ZeroMemory(&noiseRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
    noiseRVDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
    noiseRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    noiseRVDesc.Texture2D.MipLevels = texDesc.MipLevels;
    noiseRVDesc.Texture2D.MostDetailedMip = 0;

    hr = context_.d3dDevice_->CreateShaderResourceView(noiseBuffer, &noiseRVDesc, noiseRV_.GetAddressOf());
    if (FAILED(hr))
        return hr;

    noiseBuffer->Release();

    ssaoKernel_ = generateKernel();

    // ====================
    // Other initialization
    // ====================
    std::string path = "models/nanosuit/nanosuit.obj";
    model_ = std::make_unique<Models::Model>(context_, path);

    hr = reloadShaders();
    if (FAILED(hr))
        return hr;

    pointSampler_ = std::make_unique<PointWrapSampler>(context_.d3dDevice_);

    quad_ = std::make_unique<Quad>(context_.d3dDevice_);

    modelTransform_ = Transform({}, { XMConvertToRadians(0.0f), XMConvertToRadians(30.0f), XMConvertToRadians(0.0f)});

    camera_.positionCamera(
        {5.794f, 16.025f, 3.253f},
        {0.0f, 1.0f, 0.0f},
        30.0f,
        30.0f
    );

    Text::makeDefaultSDFFont(context_, font_);
    infoText_ = std::make_unique<Text::TextSDF>("PLACEHOLDER TEXT", &font_);

    return S_OK;
}

bool SSAO::SSAOExample::reloadShadersInternal() {
    return
        Shaders::makeShader<GShader>(gShader_, context_.d3dDevice_, "shaders/SSAO_GBuffer.fx", "VS", "shaders/SSAO_GBuffer.fx", "PS", Layouts::POS_NORM_UV_LAYOUT)
        && Shaders::makeShader<GBufferDisplayShader>(gBufferDisplayShader_, context_.d3dDevice_, "shaders/GBufferQuadShader.fx", "VS", "shaders/GBufferQuadShader.fx", "PS", Layouts::POS_UV_LAYOUT)
        && Shaders::makeShader<SSAOShader>(ssaoShader_, context_.d3dDevice_, "shaders/SSAO.fx", "VS", "shaders/SSAO.fx", "PS", Layouts::POS_UV_LAYOUT)
        && Shaders::makeShader<SSAOBlurShader>(ssaoBlurShader_, context_.d3dDevice_, "shaders/SSAO_Blur.fx", "VS", "shaders/SSAO_Blur.fx", "PS", Layouts::POS_UV_LAYOUT)
        && Shaders::makeShader<SSAOLightShader>(ssaoLightShader_, context_.d3dDevice_, "shaders/SSAO_Light.fx", "VS", "shaders/SSAO_Light.fx", "PS", Layouts::POS_UV_LAYOUT);
}

std::vector<XMFLOAT2> SSAO::SSAOExample::generateNoise() {
    std::vector<XMFLOAT2> data;
    for (int i = 0; i < 16; ++i) {
        data.emplace_back(randomFloats_(generator_) * 2.0f - 1.0f, randomFloats_(generator_) * 2.0f - 1.0f);
    }

    return data;
}

std::vector<XMFLOAT4> SSAO::SSAOExample::generateKernel() {
    std::vector<XMFLOAT4> data;
    for (int i = 0; i < 64; ++i) {
        XMFLOAT4 sample(randomFloats_(generator_) * 2.0f - 1.0f, randomFloats_(generator_) * 2.0f - 1.0f, randomFloats_(generator_), 1.0f);
        XMVECTOR sampleVector = XMLoadFloat4(&sample);
        sampleVector = XMVector3Normalize(sampleVector);
        sampleVector = XMVectorScale(sampleVector, randomFloats_(generator_));

        // Scale samples - they're more aligned to the center of the kernel
        float scale = float(i) / 64.0f;
        scale = Util::lerp(0.1f, 1.0f, scale * scale);
        sampleVector = XMVectorScale(sampleVector, scale);
        XMStoreFloat4(&sample, sampleVector);
        data.push_back(sample);
    }

    return data;
}

void SSAO::SSAOExample::handleInput() {
    BaseExample::handleInput();

    if (GetAsyncKeyState(toggleSSAOKey_) & 1) {
        isSSAOOn_ = !isSSAOOn_;
    }

    if (GetAsyncKeyState(incSSAOKernelSize_) & 1) {
        ssaoKernelSize += 4;
        if (ssaoKernelSize > 64) ssaoKernelSize = 64;
    }

    if (GetAsyncKeyState(decSSAOKernelSize_) & 1) {
        ssaoKernelSize -= 4;
        if (ssaoKernelSize < 4) ssaoKernelSize = 4;
    }

    if (GetAsyncKeyState(toggleSSAOKernelRotation) & 1) {
        randomRotation = (randomRotation + 1) % 2;
    }

    if (GetAsyncKeyState(toggleSSAOBlur) & 1) {
        ssaoBlur = (ssaoBlur + 1) % 2;
    }

}

void SSAO::SSAOExample::updateInfoText() const {
    using std::to_string;
    infoText_->setText(
        "\n Displayed render targets from top:\n SSAO filtered, SSAO, Albedo, Normal in view space, Position in view space\n " 
        + to_string(toggleSSAOKey_) + ": toggle SSAO"
        + "\n " + to_string(toggleSSAOKernelRotation) + ": toggle random kernel rotation"
        + "\n " + to_string(incSSAOKernelSize_) + " / " + to_string(decSSAOKernelSize_) + ": +/- SSAO kernel size"
        + "\n " + to_string(toggleSSAOBlur) + ": toggle SSAO blur"
        + "\n"
        + "\n SSAO is " + (isSSAOOn_ ? "on" : "off")
        + "\n SSAO Kernel rotation is " + (randomRotation == 1 ? "on" : "off") + "; SSAO blur is " + (ssaoBlur == 1 ? "on" : "off")
        + "\n SSAO Kernel size: " + to_string(ssaoKernelSize)
    );
}

void SSAO::SSAOExample::drawGBufferDisplays() const {
    std::array<ID3D11ShaderResourceView*, 5> gBufferViews = {
        gPositionRV_.Get(),
        gNormalRV_.Get(),
        gAlbedoRV_.Get(),
        ssaoRV_.Get(),
        ssaoBlurRV_.Get()
    };

    const float mapDisplaySize = 0.2f;

    for (size_t i = 0; i < gBufferViews.size(); ++i) {
        Transform shadowMapDisplayTransform(
            XMFLOAT3(1 - mapDisplaySize, -1 + mapDisplaySize * (i * 2 + 1), 0),
            XMFLOAT3(0, 0, 0),
            XMFLOAT3(mapDisplaySize, mapDisplaySize, mapDisplaySize)
        );
        GBufferDisplayCB gbdcb{};
        gbdcb.World = XMMatrixTranspose(shadowMapDisplayTransform.generateModelMatrix());

        gBufferDisplayShader_->use(context_.immediateContext_);
        gBufferDisplayShader_->updateConstantBuffer(context_.immediateContext_, gbdcb);
        context_.immediateContext_->PSSetShaderResources(0, 1, &gBufferViews[i]);
        pointSampler_->use(context_.immediateContext_, 0);
        quad_->draw(context_);
    }
}

void SSAO::SSAOExample::render() {
    BaseExample::render();

    // ===========================================
    // Render infromation to geometry buffer views
    // ===========================================
    std::array<ID3D11RenderTargetView*, 3> views = {
        gPositionView_.Get(),
        gNormalView_.Get(),
        gAlbedoView_.Get()
    };

    // Set multiple rendering targets
    context_.immediateContext_->OMSetRenderTargets(static_cast<UINT>(views.size()), views.data(), depthBufferDepthView_.Get());
    for (auto& view : views) {
        context_.immediateContext_->ClearRenderTargetView(view, Colors::Black);
    }
    context_.immediateContext_->ClearDepthStencilView(depthBufferDepthView_.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    GShaderCB gscb{};
    gscb.View = XMMatrixTranspose(camera_.getViewMatrix());
    gscb.Projection = XMMatrixTranspose(projection_);

    gShader_->use(context_.immediateContext_);
    gscb.World = XMMatrixTranspose(modelTransform_.generateModelMatrix());
    gscb.NormalMatrix = XMMatrixTranspose(computeNormalMatrix({ gscb.World, gscb.View }));
    gShader_->updateConstantBuffer(context_.immediateContext_, gscb);
    model_->draw(context_);

    
    // =====================
    // Generate SSAO texture
    // =====================
    context_.immediateContext_->OMSetRenderTargets(1, ssaoRTView_.GetAddressOf(), depthBufferDepthView_.Get());
    context_.immediateContext_->ClearRenderTargetView(ssaoRTView_.Get(), Colors::Black);
    context_.immediateContext_->ClearDepthStencilView(depthBufferDepthView_.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
    
    context_.immediateContext_->PSSetShaderResources(0, 1, gPositionRV_.GetAddressOf());
    context_.immediateContext_->PSSetShaderResources(1, 1, gNormalRV_.GetAddressOf());
    context_.immediateContext_->PSSetShaderResources(2, 1, noiseRV_.GetAddressOf());
    pointSampler_->use(context_.immediateContext_, 0);

    SSAOCB ssaocb{};
    ssaocb.Projection = XMMatrixTranspose(projection_);
    for (int i = 0; i < 64; ++i) {
        ssaocb.Kernel[i] = ssaoKernel_[i];
    }
    ssaocb.ScreenResolution = XMFLOAT4(static_cast<float>(context_.WIDTH), static_cast<float>(context_.HEIGHT), 0, 0);
    ssaocb.kernelSize = ssaoKernelSize;
    ssaocb.randomRotation = randomRotation;

    ssaoShader_->updateConstantBuffer(context_.immediateContext_, ssaocb);
    ssaoShader_->use(context_.immediateContext_);
    quad_->draw(context_);


    // ======================
    // Filter the SSAO buffer
    // ======================
    context_.immediateContext_->OMSetRenderTargets(1, ssaoBlurRTView_.GetAddressOf(), depthBufferDepthView_.Get());
    context_.immediateContext_->ClearRenderTargetView(ssaoBlurRTView_.Get(), Colors::Black);
    context_.immediateContext_->ClearDepthStencilView(depthBufferDepthView_.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    SSAOBlurCB ssaobcb;
    ssaobcb.blur = ssaoBlur;

    context_.immediateContext_->PSSetShaderResources(0, 1, ssaoRV_.GetAddressOf());
    pointSampler_->use(context_.immediateContext_, 0);

    ssaoBlurShader_->updateConstantBuffer(context_.immediateContext_, ssaobcb);
    ssaoBlurShader_->use(context_.immediateContext_);
    quad_->draw(context_);
    

    // =================
    // Render final quad
    // =================
    context_.immediateContext_->OMSetRenderTargets(1, &context_.renderTargetView_, context_.depthStencilView_);
    context_.immediateContext_->ClearRenderTargetView(context_.renderTargetView_, Colors::Black);
    context_.immediateContext_->ClearDepthStencilView(context_.depthStencilView_, D3D11_CLEAR_DEPTH, 1.0f, 0);
    // Set geometry buffer data as input to deferred lighting pass
    context_.immediateContext_->PSSetShaderResources(0, 1, gPositionRV_.GetAddressOf());
    context_.immediateContext_->PSSetShaderResources(1, 1, gNormalRV_.GetAddressOf());
    context_.immediateContext_->PSSetShaderResources(2, 1, gAlbedoRV_.GetAddressOf());
    context_.immediateContext_->PSSetShaderResources(3, 1, ssaoBlurRV_.GetAddressOf());

    pointSampler_->use(context_.immediateContext_, 0);

    SSAOLightCB lightCb{};
    lightCb.IsSSAOOn = isSSAOOn_ ? XMFLOAT4(1.0f, 1.0f, 1.0f, 0.0f) : XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
    lightCb.LightCol = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    lightCb.LightPos = XMFLOAT4(-30.0f, 30.0f, -30.0f, 1.0f);
    const auto lightPos = XMLoadFloat4(&lightCb.LightPos);
    XMStoreFloat4(&lightCb.LightPos, XMVector4Transform(lightPos, camera_.getViewMatrix()));
    ssaoLightShader_->updateConstantBuffer(context_.immediateContext_, lightCb);
    ssaoLightShader_->use(context_.immediateContext_);

    quad_->draw(context_);

    // Unbind GBuffer from use so we can bind it as RTV without driver forceful unbind
    static ID3D11ShaderResourceView* nullViews[] = { nullptr, nullptr, nullptr };
    context_.immediateContext_->PSSetShaderResources(0, 3, nullViews);

    // =======================
    // Forward pass for the UI
    // =======================
    // Clear depth for the UI
    context_.immediateContext_->ClearDepthStencilView(context_.depthStencilView_, D3D11_CLEAR_DEPTH, 1.0f, 0);
    drawGBufferDisplays();
    updateInfoText();
    infoText_->draw(context_);

    context_.swapChain_->Present(0, 0);
}
