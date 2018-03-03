#pragma once
#include <DirectXMath.h>

namespace VertexTypes {

struct PosVertex {
    DirectX::XMFLOAT3 Pos;
};

struct PosColVertex {
    DirectX::XMFLOAT3 Pos;
    DirectX::XMFLOAT4 Color;
};

struct PosTexVertex {
    DirectX::XMFLOAT3 Pos;
    DirectX::XMFLOAT2 UV;
};

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
