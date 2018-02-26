#pragma once
#include "DrawableObject.h"
#include "VertexTypes.h"

class Triangle : public DrawableObject<VertexTypes::PosVertex> {
public:
    explicit Triangle(ID3D11Device* device) {
        // Specify the vertices
        const std::vector<VertexTypes::PosVertex> vertices = {
            { DirectX::XMFLOAT3(0.0f, 0.5f, 0.5f) },
            { DirectX::XMFLOAT3(0.5f, -0.5f, 0.5f) },
            { DirectX::XMFLOAT3(-0.5f, -0.5f, 0.5f) },
        };

        // Specify the indices
        const std::vector<WORD> indices = {
            0, 1, 2
        };

        // DrawableObject will initialize all the buffers for us
        initialize(device, vertices, indices);
    }
};
