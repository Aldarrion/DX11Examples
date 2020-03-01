#pragma once
#include "BaseExample.h"
#include "PhongLights.h"
#include "ShaderProgram.h"
#include "Texture.h"
#include "AnisotropicSampler.h"
#include "PointWrapSampler.h"
#include "TexturedCube.h"
#include "PlaneTangentBitangent.h"
#include "WinKeyMap.h"
#include "TextSDF.h"

namespace NormalMap {

constexpr size_t NUM_LIGHTS = 2;

struct NormalMapCB {
    DirectX::XMMATRIX World;
    DirectX::XMMATRIX View;
    DirectX::XMMATRIX Projection;
    DirectX::XMMATRIX NormalMatrix;
    PointLight PointLight[NUM_LIGHTS];
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

    std::unique_ptr<TexturedCube> cube_;
    std::unique_ptr<PlaneTangentBitangent> plane_;

    Text::FontSDF font_;
    std::unique_ptr<Text::TextSDF> infoText_;

    WinKeyMap::WinKeyMap toggleRotationKey_ = WinKeyMap::E;
    bool isRotationOn_ = false;
    float rotation_ = 0.0f;

    HRESULT setup() override;
    bool reloadShadersInternal() override;
    void handleInput() override;
    void render() override;
};

}
