#include "ShadowsExample.h"
#include <directxcolors.h>
#include <minwinbase.h>
#include "Layouts.h"
#include "Transform.h"
#include "WinKeyMap.h"

using namespace DirectX;

namespace Shadows {

ShadowsExample::~ShadowsExample() {
    if (shadowMapDepthView_) 
        shadowMapDepthView_->Release();
    if (shadowShaderResourceView_) 
        shadowShaderResourceView_->Release();
}

HRESULT ShadowsExample::setup() {
    auto hr = BaseExample::setup();
    if (FAILED(hr)) 
        return hr;

    // Objects
    texturedCube_ = std::make_unique<TexturedCube>(context_.d3dDevice_);
    colorCube_ = std::make_unique<ColorCube>(context_.d3dDevice_);
    plane_ = std::make_unique<Plane>(context_.d3dDevice_);
    shadowMapDisplay_ = std::make_unique<Quad>(context_.d3dDevice_);
    Text::makeDefaultSDFFont(context_, font_);
    infoText_ = std::make_unique<Text::TextSDF>(
        "\n E: toggle rendering cube at camera's position\n Q: toggle rendering scene from light's position",
        &font_
    );

    hr = reloadShaders();
    if (FAILED(hr)) 
        return hr;

    // Textures
    seaFloorTexture_ = std::make_unique<Texture>(context_.d3dDevice_, context_.immediateContext_, L"textures/seafloor.dds", true);
    woodBoxTexture_ = std::make_unique<Texture>(context_.d3dDevice_, context_.immediateContext_, L"textures/container2.dds", true);

    // Samplers
    anisoSampler_ = std::make_unique<AnisotropicSampler>(context_.d3dDevice_);
    // Shadows need point sampler, filtering needs to be done afterwards.
    // Averaging depths would do no good
    shadowSampler_ = std::make_unique<ShadowSampler>(context_.d3dDevice_);
    pointSampler_ = std::make_unique<PointWrapSampler>(context_.d3dDevice_);

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

    ID3D11Texture2D* shadowMap = nullptr;
    hr = context_.d3dDevice_->CreateTexture2D(&texDesc, nullptr, &shadowMap);
    if (FAILED(hr)) 
        return hr;

    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory(&descDSV, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
    // Put all precision to the depth
    descDSV.Format = DXGI_FORMAT_D32_FLOAT;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;

    hr = context_.d3dDevice_->CreateDepthStencilView(shadowMap, &descDSV, &shadowMapDepthView_);
    if (FAILED(hr))
        return hr;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    // In shader sample it like regular texture with single red channel
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;

    hr = context_.d3dDevice_->CreateShaderResourceView(shadowMap, &srvDesc, &shadowShaderResourceView_);
    if (FAILED(hr)) 
        return hr;

    // The views have their own reference and since we don't need the texture anymore we can just release it
    // (decrease refcount) to avoid leaks on example exit
    shadowMap->Release();

    return S_OK;
}

bool ShadowsExample::reloadShadersInternal() {
    return
        Shaders::makeShader<ShadowShader>(shadowShader_, context_.d3dDevice_, "shaders/Shadows.fx", "VS_Shadow", "shaders/Shadows.fx", "PS_Shadow", Layouts::TEXTURED_LAYOUT)
        && Shaders::makeShader<TextureShader>(texturedPhong_, context_.d3dDevice_, "shaders/PhongShadows.fx", "VS", "shaders/PhongShadows.fx", "PS", Layouts::TEXTURED_LAYOUT)
        && Shaders::makeSolidShader(solidShader_, context_)
        && Shaders::makeShader<ShadowDisplayShader>(shadowMapDisplayShader_, context_.d3dDevice_, "shaders/ShadowMapQuadShader.fx", "VS", "shaders/ShadowMapQuadShader.fx", "PS", Layouts::POS_UV_LAYOUT);
}

void ShadowsExample::handleInput() {
    BaseExample::handleInput();
    if (GetAsyncKeyState(WinKeyMap::E) & 1) {
        isSelfCubeActive_ = !isSelfCubeActive_;
    }
    if (GetAsyncKeyState(WinKeyMap::Q) & 1) {
        drawFromLightView_ = !drawFromLightView_;
    }
}

void ShadowsExample::render() {
    BaseExample::render();

    currentCubeRotation_ += deltaTime_ * cubeRotationPerSecond_;
    while (currentCubeRotation_ > 360.0f) {
        currentCubeRotation_ -= 360.0f;
    }

    const XMFLOAT3 sunPos = XMFLOAT3(-30.0f, 30.0f, -30.0f);
    const XMMATRIX lightProjection = XMMatrixOrthographicLH(40.0f, 40.0f, 1.0f, 100.0f);
    const auto focus = XMFLOAT3(0, 0, 0);
    const auto up = XMFLOAT3(0, 1, 0);
    const XMMATRIX lightView = XMMatrixLookAtLH(XMLoadFloat3(&sunPos), XMLoadFloat3(&focus), XMLoadFloat3(&up));
    const Transform planeTransform(XMFLOAT3(0.0, -3.0f, 0.0f), XMFLOAT3(), XMFLOAT3(20.0f, 1.0f, 20.0f));
    std::vector<Transform> cubes = {
        Transform(),
        Transform(XMFLOAT3(2.5f, 1.0f, 1.8f), XMFLOAT3(XMConvertToRadians(45.0f), XMConvertToRadians(currentCubeRotation_), 0.0f)),
        Transform(XMFLOAT3(-2.5f, -1.0f, -2.5f)),
    };

    if (isSelfCubeActive_) {
        cubes.push_back(Transform(camera_.Position));
    }

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
            cb.World = XMMatrixTranspose(transform.generateModelMatrix());
            shadowShader_->updateConstantBuffer(context_.immediateContext_, cb);
            texturedCube_->draw(context_);
        }
        // Draw floor
        cb.World = XMMatrixTranspose(planeTransform.generateModelMatrix());

        shadowShader_->updateConstantBuffer(context_.immediateContext_, cb);
        plane_->draw(context_);
    }

    // ==============
    // Draw the scene
    // ==============
    {
        context_.immediateContext_->OMSetRenderTargets(1, &context_.renderTargetView_, context_.depthStencilView_);
        context_.immediateContext_->ClearRenderTargetView(context_.renderTargetView_, Util::srgbToLinear(DirectX::Colors::MidnightBlue));
        context_.immediateContext_->ClearDepthStencilView(context_.depthStencilView_, D3D11_CLEAR_DEPTH, 1.0f, 0);
        context_.immediateContext_->RSSetViewports(1, &context_.viewPort_);

        // ===========================
        // Draw the shadow map display
        // ===========================
        const float mapDisplaySize = 0.2f;
        Transform shadowMapDisplayTransform(
            XMFLOAT3(1 - mapDisplaySize, -1 + mapDisplaySize * context_.getAspectRatio(), 0),
            XMFLOAT3(0, 0, 0),
            XMFLOAT3(mapDisplaySize, mapDisplaySize * context_.getAspectRatio(), mapDisplaySize)
        );
        ShadowDisplayBuffer sdcb;
        sdcb.World = XMMatrixTranspose(shadowMapDisplayTransform.generateModelMatrix());

        shadowMapDisplayShader_->use(context_.immediateContext_);
        shadowMapDisplayShader_->updateConstantBuffer(context_.immediateContext_, sdcb);
        context_.immediateContext_->PSSetShaderResources(0, 1, &shadowShaderResourceView_);
        pointSampler_->use(context_.immediateContext_, 0);
        shadowMapDisplay_->draw(context_);

        // =========
        // Draw cube
        // =========
        ConstantBuffer cb;
        if (drawFromLightView_) {
            cb.Projection = XMMatrixTranspose(lightProjection);
            cb.View = XMMatrixTranspose(lightView);
            cb.ViewPos = sunPos;
        } else {
            cb.Projection = XMMatrixTranspose(projection_);
            cb.View = XMMatrixTranspose(camera_.getViewMatrix());
            cb.ViewPos = camera_.Position;
        }
        
        cb.LightView = XMMatrixTranspose(lightView);
        cb.LightProjection = XMMatrixTranspose(lightProjection);
        cb.SunLight.Color = SUN_YELLOW;
        cb.SunLight.Direction = XMFLOAT4(-sunPos.x, -sunPos.y, -sunPos.z, 1.0f);

        texturedPhong_->use(context_.immediateContext_);
        woodBoxTexture_->use(context_.immediateContext_, 0);
        context_.immediateContext_->PSSetShaderResources(1, 1, &shadowShaderResourceView_);
        anisoSampler_->use(context_.immediateContext_, 0);
        shadowSampler_->use(context_.immediateContext_, 1);

        for (const auto& transform : cubes) {
            cb.World = XMMatrixTranspose(transform.generateModelMatrix());
            cb.NormalMatrix = computeNormalMatrix(cb.World);
            texturedPhong_->updateConstantBuffer(context_.immediateContext_, cb);
            texturedCube_->draw(context_);
        }

        // ==========
        // Draw floor
        // ==========
        cb.World = XMMatrixTranspose(planeTransform.generateModelMatrix());
        cb.NormalMatrix = computeNormalMatrix(cb.World);

        seaFloorTexture_->use(context_.immediateContext_, 0);
        texturedPhong_->updateConstantBuffer(context_.immediateContext_, cb);
        plane_->draw(context_);

        // ========
        // Draw sun
        // ========
        ConstantBuffers::SolidConstBuffer scb;
        scb.OutputColor = SUN_YELLOW;
        scb.Projection = XMMatrixTranspose(projection_);
        scb.View = XMMatrixTranspose(camera_.getViewMatrix());
        const XMMATRIX scale = XMMatrixScaling(0.2f, 0.2f, 0.2f);
        scb.World = XMMatrixTranspose(scale * XMMatrixTranslationFromVector(XMLoadFloat3(&sunPos)));

        solidShader_->updateConstantBuffer(context_.immediateContext_, scb);
        solidShader_->use(context_.immediateContext_);
        colorCube_->draw(context_);

        infoText_->draw(context_);

        // Next frame texture bound as PS resource will be used as DSV, here we unbind manually to state the intent
        // Otherwise, the driver would unbind the texture forecfully, see ID3D11DeviceContext::OMSetRenderTargets on MSDN
        static ID3D11ShaderResourceView* nullViews[] = {
            nullptr, nullptr
        };
        context_.immediateContext_->PSSetShaderResources(0, 2, nullViews);

        context_.swapChain_->Present(0, 0);
    }
}
}
