#pragma once
#include "BasicExample.h"
#include "ColorCube.h"
#include "TexturedCube.h"
#include "ShaderProgram.h"
#include "PhongLights.h"
#include "ShadowSampler.h"
#include "LinearSampler.h"
#include "Texture.h"

namespace Shadows {

struct ConstantBuffer {
    DirectX::XMMATRIX World;
    DirectX::XMMATRIX View;
    DirectX::XMMATRIX Projection;
    DirectX::XMMATRIX NormalMatrix;
    DirectX::XMMATRIX LightView;
    DirectX::XMMATRIX LightProjection;
    DirLight DirLights[1];
    DirectX::XMFLOAT3 ViewPos;
    int DirLightCount;
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
    std::unique_ptr<LinearSampler> linearSampler_;
    std::unique_ptr<ShadowShader> shadowShader_;
    std::unique_ptr<TextureShader> texturedPhong_;
    std::unique_ptr<TexturedCube> texturedCube_;
    std::unique_ptr<SolidShader> solidShader_;
    std::unique_ptr<ColorCube> colorCube_;

    // Shadows
    ID3D11Texture2D* shadowMap_ = nullptr;
    ID3D11DepthStencilView* shadowMapDepthView_ = nullptr;
    ID3D11ShaderResourceView* shadowShaderResourceView_ = nullptr;

    HRESULT setup() override;
    
    void render() override;

public:
    virtual ~ShadowsExample() = default;
};
}
