#pragma once

#pragma once
#include <d3d11.h>
#include <directxmath.h>

struct FullVertex {
    DirectX::XMFLOAT3 Pos;
    DirectX::XMFLOAT3 Normal;
    DirectX::XMFLOAT3 Color;
    DirectX::XMFLOAT2 UV;
};

class TexturedCube {
private:
    ID3D11Buffer * vertexBuffer_;
    ID3D11Buffer* indexBuffer_;

public:
    explicit TexturedCube(ID3D11Device* device) {
        // Create vertex buffer
        FullVertex vertices[] = {
            { DirectX::XMFLOAT3(-1.0f, 1.0f, -1.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),     DirectX::XMFLOAT3(0.2f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
            { DirectX::XMFLOAT3(1.0f, 1.0f, -1.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),      DirectX::XMFLOAT3(0.2f, 1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },
            { DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),       DirectX::XMFLOAT3(0.2f, 1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
            { DirectX::XMFLOAT3(-1.0f, 1.0f, 1.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),      DirectX::XMFLOAT3(1.0f, 0.2f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
                                                                                                                          
            { DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f),   DirectX::XMFLOAT3(0.1f, 1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },
            { DirectX::XMFLOAT3(1.0f, -1.0f, -1.0f), DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f),    DirectX::XMFLOAT3(0.1f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
            { DirectX::XMFLOAT3(1.0f, -1.0f, 1.0f), DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f),     DirectX::XMFLOAT3(0.1f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
            { DirectX::XMFLOAT3(-1.0f, -1.0f, 1.0f), DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f),    DirectX::XMFLOAT3(0.1f, 1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
                                                                                                                             
            { DirectX::XMFLOAT3(-1.0f, -1.0f, 1.0f), DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f),    DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
            { DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f),   DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
            { DirectX::XMFLOAT3(-1.0f, 1.0f, -1.0f), DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f),    DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
            { DirectX::XMFLOAT3(-1.0f, 1.0f, 1.0f), DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f),     DirectX::XMFLOAT3(1.0f, 0.2f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },
                                                                                                                               
            { DirectX::XMFLOAT3(1.0f, -1.0f, 1.0f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f),      DirectX::XMFLOAT3(0.1f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
            { DirectX::XMFLOAT3(1.0f, -1.0f, -1.0f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f),     DirectX::XMFLOAT3(0.1f, 1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
            { DirectX::XMFLOAT3(1.0f, 1.0f, -1.0f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f),      DirectX::XMFLOAT3(0.1f, 1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },
            { DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f),       DirectX::XMFLOAT3(0.1f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
                                                                                                                               
            { DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f),   DirectX::XMFLOAT3(0.5f, 0.7f, 0.5f), DirectX::XMFLOAT2(0.0f, 1.0f) },
            { DirectX::XMFLOAT3(1.0f, -1.0f, -1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f),    DirectX::XMFLOAT3(0.5f, 0.7f, 0.5f), DirectX::XMFLOAT2(1.0f, 1.0f) },
            { DirectX::XMFLOAT3(1.0f, 1.0f, -1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f),     DirectX::XMFLOAT3(0.5f, 0.7f, 0.5f), DirectX::XMFLOAT2(1.0f, 0.0f) },
            { DirectX::XMFLOAT3(-1.0f, 1.0f, -1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f),    DirectX::XMFLOAT3(0.1f, 0.7f, 0.5f), DirectX::XMFLOAT2(0.0f, 0.0f) },
                                                                                                                               
            { DirectX::XMFLOAT3(-1.0f, -1.0f, 1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f),     DirectX::XMFLOAT3(0.1f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
            { DirectX::XMFLOAT3(1.0f, -1.0f, 1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f),      DirectX::XMFLOAT3(0.1f, 1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
            { DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f),       DirectX::XMFLOAT3(0.1f, 1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },
            { DirectX::XMFLOAT3(-1.0f, 1.0f, 1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f),      DirectX::XMFLOAT3(1.0f, 0.2f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
        };

        D3D11_BUFFER_DESC bd;
        ZeroMemory(&bd, sizeof(bd));
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(FullVertex) * 24;
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.CPUAccessFlags = 0;
        D3D11_SUBRESOURCE_DATA InitData;
        ZeroMemory(&InitData, sizeof(InitData));
        InitData.pSysMem = vertices;
        auto hr = device->CreateBuffer(&bd, &InitData, &vertexBuffer_);
        if (FAILED(hr)) {
            MessageBox(nullptr, L"Failed to create vertex buffer", L"Error", MB_OK);
            return;
        }

        // Create index buffer
        WORD indices[] = {
            3,1,0,
            2,1,3,

            6,4,5,
            7,4,6,

            11,9,8,
            10,9,11,

            14,12,13,
            15,12,14,

            19,17,16,
            18,17,19,

            22,20,21,
            23,20,22
        };
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(WORD) * 36;        // 36 vertices needed for 12 triangles in a triangle list
        bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bd.CPUAccessFlags = 0;
        InitData.pSysMem = indices;
        hr = device->CreateBuffer(&bd, &InitData, &indexBuffer_);
        if (FAILED(hr)) {
            MessageBox(nullptr, L"Failed to create index buffer", L"Error", MB_OK);
            return;
        }
    }

    ~TexturedCube() {
        if (vertexBuffer_) vertexBuffer_->Release();
        if (indexBuffer_) indexBuffer_->Release();
    }

    TexturedCube(const TexturedCube&) = delete;
    TexturedCube operator=(const TexturedCube&) = delete;

    void draw(ID3D11DeviceContext* context) const {
        UINT stride = sizeof(FullVertex);
        UINT offset = 0;
        context->IASetVertexBuffers(0, 1, &vertexBuffer_, &stride, &offset);
        context->IASetIndexBuffer(indexBuffer_, DXGI_FORMAT_R16_UINT, 0);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->DrawIndexed(36, 0, 0);
    }
};

