#pragma once
#include "BaseExample.h"
#include "ShaderProgram.h"
#include "PhongShadingExample.h"
#include "Texture.h"
#include "AnisotropicSampler.h"
#include "TexturedCube.h"
#include "Plane.h"

namespace Specular {

struct SpecularCB {
    DirectX::XMMATRIX World;
    DirectX::XMMATRIX View;
    DirectX::XMMATRIX Projection;
    DirectX::XMMATRIX NormalMatrix;
    DirLight SunLight;
    DirectX::XMFLOAT3 ViewPos;
    int UseSpecular;
};

class SpecularMapExample : public BaseExample {
protected:
    using SpecularShader = ShaderProgram<SpecularCB>;

    std::unique_ptr<SpecularShader> shader_;
    Shaders::PSolidShader solidShader_;
    Textures::PTexture seaFloorTexture_;
    Textures::PTexture boxDiffuse_;
    Textures::PTexture boxSpecular_;
    Samplers::PAnisotropicSampler sampler_;

    std::unique_ptr<TexturedCube> cube_;

    HRESULT setup() override;
    bool reloadShadersInternal() override;
    void render() override;
};
}
