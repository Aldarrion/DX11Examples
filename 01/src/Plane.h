#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include "VertexTypes.h"
#include <vector>
#include "DrawableObject.h"

class Plane : public DrawableObject<VertexTypes::FullVertex> {
public:
    explicit Plane(ID3D11Device* device) {
        static constexpr float textureScale = 15.0f;
        const std::vector<VertexTypes::FullVertex> vertices = {
            { DirectX::XMFLOAT3(-1.0f, 1.0f, -1.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), DirectX::XMFLOAT3(0.2f, 1.0f, 0.0f), DirectX::XMFLOAT2(textureScale, 0.0f) },
            { DirectX::XMFLOAT3(1.0f, 1.0f, -1.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),  DirectX::XMFLOAT3(0.2f, 1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },
            { DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),   DirectX::XMFLOAT3(0.2f, 1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, textureScale) },
            { DirectX::XMFLOAT3(-1.0f, 1.0f, 1.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),  DirectX::XMFLOAT3(1.0f, 0.2f, 0.0f), DirectX::XMFLOAT2(textureScale, textureScale) }
        };

        const std::vector<WORD> indices = {
            3,1,0,
            2,1,3
        };

        initialize(device, vertices, indices);
    }
};
