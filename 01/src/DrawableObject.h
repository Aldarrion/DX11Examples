#pragma once
#include <d3d11.h>
#include <vector>
#include "ResourceHolder.h"

/**
 * \brief Base class for simple objects specified by vertex and index buffers.
 * Inherit from this class and provide vertex and index buffers to initialize method.
 * Draw method will automatically draw the buffers you provided.
 * \tparam TVertex Type of vertex in given vertex buffer.
 */
template<typename TVertex>
class DrawableObject : public ResourceHolder {
protected:
    ID3D11Buffer* vertexBuffer_;
    ID3D11Buffer* indexBuffer_;
    UINT indexCount_;

    /**
     * \brief Call method in constructor of inheriting class to initialize this DrawableObject
     * @param device DirectX device this object should use to create buffers.
     * @param vertices List of vertices which will be used as vertex buffer.
     * @param indices List of indices which define the object to draw.
     */
    void initialize(
        ID3D11Device* device,
        const std::vector<TVertex>& vertices,
        const std::vector<WORD>& indices
    ) {
        // ====================
        // Create vertex buffer
        // ====================
        // Create description for vertex buffer
        D3D11_BUFFER_DESC vertexBuffDesc;
        ZeroMemory(&vertexBuffDesc, sizeof(vertexBuffDesc));
        vertexBuffDesc.Usage = D3D11_USAGE_DEFAULT;
        vertexBuffDesc.ByteWidth = static_cast<UINT>(sizeof(TVertex) * vertices.size());
        vertexBuffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBuffDesc.CPUAccessFlags = 0;
        
        // We have data we want to use in this buffer - create wrapper for them and initialize it
        D3D11_SUBRESOURCE_DATA vertexData;
        ZeroMemory(&vertexData, sizeof(vertexData));
        vertexData.pSysMem = vertices.data();

        // Try to create vertex buffer using given device and description
        auto hr = device->CreateBuffer(&vertexBuffDesc, &vertexData, &vertexBuffer_);
        if (FAILED(hr)) {
            MessageBox(nullptr, L"Failed to create vertex buffer", L"Error", MB_OK);
            return;
        }

        // ===================
        // Create index buffer
        // ===================
        // Create description for index buffer
        D3D11_BUFFER_DESC indexBuffDesc;
        ZeroMemory(&indexBuffDesc, sizeof(indexBuffDesc));
        indexBuffDesc.Usage = D3D11_USAGE_DEFAULT;
        indexBuffDesc.ByteWidth = static_cast<UINT>(sizeof(WORD) * indices.size());
        indexBuffDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        indexBuffDesc.CPUAccessFlags = 0;

        // We have data we want to use in this buffer - create wrapper from them and initialize it
        D3D11_SUBRESOURCE_DATA indexData;
        ZeroMemory(&vertexData, sizeof(indexData));
        indexData.pSysMem = indices.data();

        // Try to create index buffer using given device and description
        hr = device->CreateBuffer(&indexBuffDesc, &indexData, &indexBuffer_);
        if (FAILED(hr)) {
            MessageBox(nullptr, L"Failed to create index buffer", L"Error", MB_OK);
            return;
        }

        // Save index count for drawing
        indexCount_ = static_cast<UINT>(indices.size());
    }

public:
    DrawableObject() = default;

    virtual ~DrawableObject() {
        if (vertexBuffer_) vertexBuffer_->Release();
        if (indexBuffer_) indexBuffer_->Release();
    }

    /**
     * \brief Draws this DrawableObject to currently used buffer in given context.
     * @param context Device context used for drawing.
     */
    virtual void draw(ID3D11DeviceContext* context) const {
        const UINT stride = sizeof(TVertex);
        const UINT offset = 0;
        // Set vertex buffer to use for drawing
        context->IASetVertexBuffers(0, 1, &vertexBuffer_, &stride, &offset);
        // Set index buffer of 16 byte unsigned integers to use for drawing
        context->IASetIndexBuffer(indexBuffer_, DXGI_FORMAT_R16_UINT, 0);
        // Set topology - how should we interpret the indices given - to a list of triangles
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        // Draw geometry in vertex buffer using index buffer
        context->DrawIndexed(indexCount_, 0, 0);
    }
};
