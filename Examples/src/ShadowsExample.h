#pragma once
#include "BaseExample.h"
#include "ColorCube.h"
#include "TexturedCube.h"
#include "ShaderProgram.h"
#include "PhongLights.h"
#include "ShadowSampler.h"
#include "Texture.h"
#include "AnisotropicSampler.h"
#include "Plane.h"
#include "ConstantBuffers.h"
#include "Quad.h"
#include "PointWrapSampler.h"
#include "TextSDF.h"

namespace Shadows {

struct ConstantBuffer {
    DirectX::XMMATRIX World;
    DirectX::XMMATRIX View;
    DirectX::XMMATRIX Projection;
    DirectX::XMMATRIX NormalMatrix;
    DirectX::XMMATRIX LightView;
    DirectX::XMMATRIX LightProjection;
    DirLight SunLight;
    DirectX::XMFLOAT3 ViewPos;
};

struct ShadowConstBuffer {
    DirectX::XMMATRIX World;
    DirectX::XMMATRIX View;
    DirectX::XMMATRIX Projection;
};

struct ShadowDisplayBuffer {
    DirectX::XMMATRIX World;
};

class ShadowsExample : public BaseExample {
public:
    ~ShadowsExample() override;
protected:
    using TextureShader = ShaderProgram<ConstantBuffer>;
    using SolidShader = ShaderProgram<ConstantBuffers::SolidConstBuffer>;
    using ShadowShader = ShaderProgram<ShadowConstBuffer>;
    using ShadowDisplayShader = ShaderProgram<ShadowDisplayBuffer>;

    std::unique_ptr<Texture> seaFloorTexture_;
    std::unique_ptr<Texture> woodBoxTexture_;
    std::unique_ptr<ShadowSampler> shadowSampler_;
    std::unique_ptr<AnisotropicSampler> anisoSampler_;
    std::unique_ptr<ShadowShader> shadowShader_;
    std::unique_ptr<TextureShader> texturedPhong_;
    std::unique_ptr<TexturedCube> texturedCube_;
    std::unique_ptr<Plane> plane_;
    std::unique_ptr<SolidShader> solidShader_;
    std::unique_ptr<ColorCube> colorCube_;
    float currentCubeRotation_ = 0.0f;
    float cubeRotationPerSecond_ = 45.0f;

    std::unique_ptr<Quad> shadowMapDisplay_;
    std::unique_ptr<ShadowDisplayShader> shadowMapDisplayShader_;
    std::unique_ptr<PointWrapSampler> pointSampler_;

    Text::FontSDF font_;
    std::unique_ptr<Text::TextSDF> infoText_;
    bool isSelfCubeActive_ = true;
    bool drawFromLightView_ = false;

    // =======
    // Shadows
    // =======
    
    // Resolution of the shadowmap
    static constexpr UINT SHADOW_MAP_WIDTH = 1024;
    static constexpr UINT SHADOW_MAP_HEIGHT = 1024;

    ID3D11DepthStencilView* shadowMapDepthView_ = nullptr;
    ID3D11ShaderResourceView* shadowShaderResourceView_ = nullptr;
    D3D11_VIEWPORT shadowViewPort_;


    HRESULT setup() override;
    bool reloadShadersInternal() override;
    void handleInput() override;
    void render() override;
};
}
