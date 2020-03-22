#pragma once
#include "BaseExample.h"
#include "ShaderProgram.h"
#include "AnisotropicSampler.h"
#include "Quad.h"
#include "Model.h"
#include "PointWrapSampler.h"
#include "Transform.h"
#include "TextSDF.h"
#include "ColorCube.h"
#include "WinKeyMap.h"
#include "PhongLights.h"

#include <wrl/client.h>

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

class DeferredRenderingExample : public BaseExample {
protected:
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthBufferDSV_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> depthBufferSRV_;

    // Geometry buffers
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> gPositionRTV_;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> gNormalRTV_;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> gAlbedoRTV_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> gPositionSRV_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> gNormalSRV_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> gAlbedoSRV_;

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

    Shaders::PTexturedQuad gBufferDisplayShader_;

    std::unique_ptr<ColorCube> colorCube_;
    std::unique_ptr<Models::Model> model_;
    std::unique_ptr<Quad> quad_;

    Samplers::PAnisotropicSampler anisoSampler_;
    std::unique_ptr<PointWrapSampler> pointSampler_;

    std::array<PointLight, NUM_LIGHTS> lights_;
    std::vector<Transform> modelTransforms_;

    WinKeyMap::WinKeyMap switchRenderingModeKey_ = WinKeyMap::E;
    Text::FontSDF font_;
    std::unique_ptr<Text::TextSDF> infoText_;

    ContextSettings getSettings() const override;
    HRESULT setup() override;
    bool reloadShadersInternal() override;
    void handleInput() override;
    void render() override;

    void drawText() const;
    void renderLights() const;
    void drawGBufferDisplays() const;

    void renderDeferred();
    void renderForward();
};
}
