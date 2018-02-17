#pragma once
#include <d3d11.h>
#include <vector>

template<typename TVertex>
class DrawableObject {
protected:
    ID3D11Buffer* vertexBuffer_;
    ID3D11Buffer* indexBuffer_;
    UINT indexCount_;

    void initialize(
        ID3D11Device* device,
        const std::vector<TVertex>& vertices,
        const std::vector<WORD>& indices
    ) {
        D3D11_BUFFER_DESC bd;
        ZeroMemory(&bd, sizeof(bd));
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = static_cast<UINT>(sizeof(TVertex) * vertices.size());
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.CPUAccessFlags = 0;
        D3D11_SUBRESOURCE_DATA InitData;
        ZeroMemory(&InitData, sizeof(InitData));
        InitData.pSysMem = vertices.data();
        auto hr = device->CreateBuffer(&bd, &InitData, &vertexBuffer_);
        if (FAILED(hr)) {
            MessageBox(nullptr, L"Failed to create vertex buffer", L"Error", MB_OK);
            return;
        }

        // Create index buffer
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = static_cast<UINT>(sizeof(WORD) * indices.size());
        bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bd.CPUAccessFlags = 0;
        InitData.pSysMem = indices.data();
        hr = device->CreateBuffer(&bd, &InitData, &indexBuffer_);
        if (FAILED(hr)) {
            MessageBox(nullptr, L"Failed to create index buffer", L"Error", MB_OK);
            return;
        }
        indexCount_ = static_cast<UINT>(indices.size());
    }

public:
    DrawableObject() = default;

    virtual ~DrawableObject() {
        if (vertexBuffer_) vertexBuffer_->Release();
        if (indexBuffer_) indexBuffer_->Release();
    }

    DrawableObject(const DrawableObject&) = delete;
    DrawableObject operator=(const DrawableObject&) = delete;

    virtual void draw(ID3D11DeviceContext* context) const {
        const UINT stride = sizeof(TVertex);
        const UINT offset = 0;
        context->IASetVertexBuffers(0, 1, &vertexBuffer_, &stride, &offset);
        context->IASetIndexBuffer(indexBuffer_, DXGI_FORMAT_R16_UINT, 0);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->DrawIndexed(indexCount_, 0, 0);
    }
};
