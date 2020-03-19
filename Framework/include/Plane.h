#pragma once
#include "VertexTypes.h"
#include "DrawableObject.h"

#include <DirectXMath.h>
#include <vector>

class Plane : public DrawableObject<VertexTypes::FullVertex> {
public:
    explicit Plane(ID3D11Device* device, const float textureScale = 15.0f) {
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

    Layouts::VertexLayout_t getVertexLayout() const override {
        return Layouts::TEXTURED_LAYOUT;
    }

    const wchar_t* getObjectName() const override {
        return L"Plane";
    }
};
