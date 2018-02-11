#pragma once
#include "BasicExample.h"
#include "ColorCube.h"
#include "TexturedCube.h"
#include "ShaderProgram.h"
#include "PhongLights.h"
#include "ShadowSampler.h"
#include "Texture.h"
#include "AnisotropicSampler.h"

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

struct SolidConstBuffer {
    DirectX::XMMATRIX World;
    DirectX::XMMATRIX View;
    DirectX::XMMATRIX Projection;
    DirectX::XMFLOAT4 OutputColor;
};

struct ShadowConstBuffer {
    DirectX::XMMATRIX World;
    DirectX::XMMATRIX View;
    DirectX::XMMATRIX Projection;
};

class ShadowsExample : public BasicExample {
protected:
    using TextureShader = ShaderProgram<ConstantBuffer>;
    using SolidShader = ShaderProgram<SolidConstBuffer>;
    using ShadowShader = ShaderProgram<ShadowConstBuffer>;

    std::unique_ptr<Texture> seaFloorTexture_;
    std::unique_ptr<ShadowSampler> shadowSampler_;
    std::unique_ptr<AnisotropicSampler> anisoSampler_;
    std::unique_ptr<ShadowShader> shadowShader_;
    std::unique_ptr<TextureShader> texturedPhong_;
    std::unique_ptr<TexturedCube> texturedCube_;
    std::unique_ptr<SolidShader> solidShader_;
    std::unique_ptr<ColorCube> colorCube_;
    float currentCubeRotation_ = 0.0f;
    float cubeRotationPerSecond_ = 45.0f;

    // =======
    // Shadows
    // =======
    
    // Resolution of the shadowmap
    static constexpr UINT SHADOW_MAP_WIDTH = 1024;
    static constexpr UINT SHADOW_MAP_HEIGHT = 1024;

    ID3D11Texture2D* shadowMap_ = nullptr;
    ID3D11DepthStencilView* shadowMapDepthView_ = nullptr;
    ID3D11ShaderResourceView* shadowShaderResourceView_ = nullptr;
    D3D11_VIEWPORT shadowViewPort_;


    HRESULT setup() override;
    
    void render() override;

public:
    virtual ~ShadowsExample() = default;
};
}
