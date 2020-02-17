#pragma once
#include <directxmath.h>

constexpr DirectX::XMFLOAT4 SUN_YELLOW = DirectX::XMFLOAT4(0.792f, 0.772f, 0.655f, 1.0f);

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

namespace ConstantBuffers {
struct PhongCB {
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
    float Shininess;
};
}

