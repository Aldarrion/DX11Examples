#pragma once
#include "BaseExample.h"
#include "ShaderProgram.h"
#include "AnisotropicSampler.h"
#include "Quad.h"
#include "Model.h"
#include "PointWrapSampler.h"
#include "Transform.h"
#include "Text.h"
#include "ColorCube.h"
#include "WinKeyMap.h"

namespace Deferred {

struct GShaderCB {
    DirectX::XMMATRIX World;
    DirectX::XMMATRIX View;
    DirectX::XMMATRIX Projection;
    DirectX::XMMATRIX NormalMatrix;
};

struct UnlitCB {
    DirectX::XMMATRIX World;
    DirectX::XMMATRIX View;
    DirectX::XMMATRIX Projection;
    DirectX::XMMATRIX NormalMatrix;
};

constexpr size_t NUM_LIGHTS = 256;

struct DeferredLightCB {
    PointLight Lights[NUM_LIGHTS];
    DirectX::XMFLOAT4 ViewPos;
};

struct ForwardCB {
    DirectX::XMMATRIX World;
    DirectX::XMMATRIX View;
    DirectX::XMMATRIX Projection;
    DirectX::XMMATRIX NormalMatrix;
    PointLight Lights[NUM_LIGHTS];
    DirectX::XMFLOAT3 ViewPos;
};

struct GBufferDisplayCB {
    DirectX::XMMATRIX World;
};

class DeferredRenderingExample : public BaseExample {
protected:
    ID3D11Texture2D * depthBuffer_ = nullptr;
    ID3D11DepthStencilView* depthBufferDepthView_ = nullptr;
    ID3D11ShaderResourceView* depthBufferResourceView_ = nullptr;

    // Geometry buffers
    ID3D11Texture2D * gPosition_ = nullptr;
    ID3D11Texture2D* gNormal_ = nullptr;
    ID3D11Texture2D* gAlbedo_ = nullptr;
    ID3D11RenderTargetView* gPositionView_ = nullptr;
    ID3D11RenderTargetView* gNormalView_ = nullptr;
    ID3D11RenderTargetView* gAlbedoView_ = nullptr;
    ID3D11ShaderResourceView* gPositionRV_ = nullptr;
    ID3D11ShaderResourceView* gNormalRV_ = nullptr;
    ID3D11ShaderResourceView* gAlbedoRV_ = nullptr;

    bool isDeferredRendering_ = true;

    // Shaders
    using GShader = ShaderProgram<GShaderCB>;
    using UnlitShader = ShaderProgram<UnlitCB>;
    using PGShader = std::unique_ptr<GShader>;
    using DefferedShader = ShaderProgram<DeferredLightCB>;
    using PDeferredShader = std::unique_ptr<DefferedShader>;

    PGShader gShader_;
    PDeferredShader defferedLightShader_;

    using ForwardShader = ShaderProgram<ForwardCB>;
    using PForwardShader = std::unique_ptr<ForwardShader>;

    PForwardShader forwardShader_;
    Shaders::PSolidShader lightShader_;

    using GBufferDisplayShader = ShaderProgram<GBufferDisplayCB>;
    using PGBufferDisplayShader = std::unique_ptr<GBufferDisplayShader>;

    PGBufferDisplayShader gBufferDisplayShader_;

    std::unique_ptr<ColorCube> colorCube_;
    std::unique_ptr<Models::Model> model_;
    std::unique_ptr<Quad> quad_;

    Samplers::PAnisotropicSampler anisoSampler_;
    std::unique_ptr<PointWrapSampler> pointSampler_;

    std::array<PointLight, NUM_LIGHTS> lights_;
    std::vector<Transform> modelTransforms_;

    WinKeyMap::WinKeyMap switchRenderingModeKey_ = WinKeyMap::R;
    std::unique_ptr<Text::Text> infoText_;

    ContextSettings getSettings() const override;
    HRESULT setup() override;
    void handleInput() override;
    void render() override;

    void drawText() const;
    void renderLights() const;
    void drawGBufferDisplays() const;

    void renderDeferred();
    void renderForward();
};
}
