#pragma once
#include "BasicExample.h"
#include "ShaderProgram.h"
#include "ColorCube.h"

struct PointLight {
    DirectX::XMFLOAT4 Position;
    DirectX::XMFLOAT4 Color;
};

struct DirLight {
    DirectX::XMFLOAT4 Direction;
    DirectX::XMFLOAT4 Color;
};

struct SpotLight {
    DirectX::XMFLOAT4 Position;
    DirectX::XMFLOAT4 Direction;
    DirectX::XMFLOAT4 Color;
    DirectX::XMFLOAT4 InnerCone;
    DirectX::XMFLOAT4 OuterCone;
};

struct ConstantBuffer {
    DirectX::XMMATRIX mWorld;
    DirectX::XMMATRIX mView;
    DirectX::XMMATRIX mProjection;
    PointLight PointLights[2];
    DirLight DirLights[2];
    SpotLight SpotLights[2];
    DirectX::XMFLOAT3 vViewPos;
    int PointLightCount;
    int DirLightCount;
    int SpotLightCount;
};

struct SolidConstBuffer {
    DirectX::XMMATRIX mWorld;
    DirectX::XMMATRIX mView;
    DirectX::XMMATRIX mProjection;
    DirectX::XMFLOAT4 vOutputColor;
};

class PhongShadingExample : public BasicExample {
    DirectX::XMMATRIX world_;
    DirectX::XMMATRIX projection_;

    std::unique_ptr<ShaderProgram<ConstantBuffer>> cubeShader_;
    std::unique_ptr<ShaderProgram<SolidConstBuffer>> solidShader_;
    std::unique_ptr<ColorCube> colorCube_;

    HRESULT setup() override;
    void render() override;

public:
    virtual ~PhongShadingExample() = default;
};
