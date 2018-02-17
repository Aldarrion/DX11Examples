#pragma once
#include "BaseExample.h"
#include "ShaderProgram.h"
#include "ColorCube.h"
#include "PhongLights.h"

struct ConstantBuffer {
    DirectX::XMMATRIX World;
    DirectX::XMMATRIX View;
    DirectX::XMMATRIX Projection;
    DirectX::XMMATRIX NormalMatrix;
    PointLight PointLights[2];
    DirLight DirLights[2];
    SpotLight SpotLights[2];
    DirectX::XMFLOAT3 ViewPos;
    int PointLightCount;
    int DirLightCount;
    int SpotLightCount;
};

struct SolidConstBuffer {
    DirectX::XMMATRIX World;
    DirectX::XMMATRIX View;
    DirectX::XMMATRIX Projection;
    DirectX::XMFLOAT4 OutputColor;
};

class PhongShadingExample : public BaseExample {
protected:
    std::unique_ptr<ShaderProgram<ConstantBuffer>> cubeShader_;
    std::unique_ptr<ShaderProgram<SolidConstBuffer>> solidShader_;
    std::unique_ptr<ColorCube> colorCube_;

    HRESULT setup() override;
    void render() override;

public:
    virtual ~PhongShadingExample() = default;
};
