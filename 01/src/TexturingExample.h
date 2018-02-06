#pragma once
#include "BasicExample.h"

#include "PhongLights.h"
#include "ShaderProgram.h"
#include "TexturedCube.h"
#include "ColorCube.h"


namespace Texturing {

struct ConstantBuffer {
    DirectX::XMMATRIX World;
    DirectX::XMMATRIX View;
    DirectX::XMMATRIX Projection;
    DirectX::XMMATRIX NormalMatrix;
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

class TexturingExample : public BasicExample {
protected:
    using TextureShader = ShaderProgram<ConstantBuffer>;
    using SolidShader = ShaderProgram<SolidConstBuffer>;

    ID3D11ShaderResourceView* seaFloorTexture_ = nullptr;
    ID3D11SamplerState* textureSampler_ = nullptr;

    std::unique_ptr<TextureShader> texturedPhong_;
    std::unique_ptr<TexturedCube> texturedCube_;
    std::unique_ptr<SolidShader> solidShader_;
    std::unique_ptr<ColorCube> colorCube_;

    HRESULT setup() override;
    void render() override;
};
}
