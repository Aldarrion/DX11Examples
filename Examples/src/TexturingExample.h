#pragma once
#include "BaseExample.h"

#include "PhongLights.h"
#include "ShaderProgram.h"
#include "TexturedCube.h"
#include "ColorCube.h"
#include "Plane.h"

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

class TexturingExample : public BaseExample {
public:
    ~TexturingExample() override;

protected:
    using TextureShader = ShaderProgram<ConstantBuffer>;

    ID3D11Resource* seaFloorTexResource_{ nullptr };
    ID3D11ShaderResourceView* seaFloorTexture_{ nullptr };
    ID3D11Resource* boxTexResource_{ nullptr };
    ID3D11ShaderResourceView* boxTexture_{ nullptr };
    ID3D11SamplerState* textureSampler_{ nullptr };

    std::unique_ptr<TextureShader> texturedPhong_;
    std::unique_ptr<TexturedCube> texturedCube_;
    std::unique_ptr<Plane> texturedPlane_;
    Shaders::PSolidShader solidShader_;
    std::unique_ptr<ColorCube> colorCube_;

    HRESULT setup() override;
    bool reloadShadersInternal() override;
    void render() override;
};
}
