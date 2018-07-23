#pragma once
#include "BaseExample.h"
#include "PhongLights.h"
#include "ShaderProgram.h"
#include "Texture.h"
#include "AnisotropicSampler.h"
#include "PointWrapSampler.h"
#include "TexturedCube.h"
#include "PlaneTangentBitangent.h"

namespace NormalMap {

struct NormalMapCB {
    DirectX::XMMATRIX World;
    DirectX::XMMATRIX View;
    DirectX::XMMATRIX Projection;
    DirectX::XMMATRIX NormalMatrix;
    DirLight SunLight;
    DirectX::XMFLOAT3 ViewPos;
    int UseNormalMap;
};

class NormalMappingExample : public BaseExample {
private:
    using NormalMapShader = ShaderProgram<NormalMapCB>;
    using PNormalMapShader = std::unique_ptr<NormalMapShader>;

    PNormalMapShader normalMapShader_;
    Shaders::PSolidShader solidShader_;
    Textures::PTexture wallDiffuse_;
    Textures::PTexture wallNormalMap_;
    Samplers::PAnisotropicSampler anisoSampler_;
    Samplers::PPointWrapSampler pointWrapSampler_;

    std::unique_ptr<TexturedCube> cube_;
    std::unique_ptr<PlaneTangentBitangent> plane_;

public:
    HRESULT setup() override;
    void handleInput() override;
    void render() override;
};

}
