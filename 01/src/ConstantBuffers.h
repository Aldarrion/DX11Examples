#pragma once
#include <DirectXMath.h>
#include "PhongLights.h"

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

struct SolidConstBuffer {
    DirectX::XMMATRIX World;
    DirectX::XMMATRIX View;
    DirectX::XMMATRIX Projection;
    DirectX::XMFLOAT4 OutputColor;
};
}
