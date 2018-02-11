#pragma once
#include <DirectXMath.h>

namespace VertexTypes {

struct SimpleVertex {
    DirectX::XMFLOAT3 Pos;
    DirectX::XMFLOAT3 Normal;
    DirectX::XMFLOAT3 Color;
};

struct FullVertex {
    DirectX::XMFLOAT3 Pos;
    DirectX::XMFLOAT3 Normal;
    DirectX::XMFLOAT3 Color;
    DirectX::XMFLOAT2 UV;
};
}
