#pragma once
#include "ResourceHolder.h"
#include "Layouts.h"
#include "ContextWrapper.h"
#include "Logging.h"

#include <vector>
#include <cassert>
#include <cwchar>

/**
 * \brief Base class for simple objects specified by vertex and index buffers.
 * Inherit from this class and provide vertex and index buffers to initialize method.
 * Draw method will automatically draw the buffers you provided.
 * \tparam TVertex Type of vertex in given vertex buffer.
 */
template<typename TVertex>
class DrawableObject : public ResourceHolder {
public:
    DrawableObject() = default;

    virtual ~DrawableObject() {
        if (vertexBuffer_) 
            vertexBuffer_->Release();
        if (indexBuffer_) 
            indexBuffer_->Release();
    }

    /**
     * \brief Draws this DrawableObject to currently used buffer in given context.
     * @param context Device context used for drawing.
     */
    virtual void draw(const ContextWrapper& context) const {
        static constexpr unsigned int BUFF_LEN = 256;
        wchar_t buff[BUFF_LEN];
        const wchar_t* prefix = L"Draw ";
        const wchar_t* objName = getObjectName();
        const size_t size = wcslen(prefix) + wcslen(objName);
        assert(size < BUFF_LEN);

        std::swprintf(buff, BUFF_LEN, L"%s%s", prefix, objName);
        
        ex::beginEvent(context.perf_, buff);
        const UINT stride = sizeof(TVertex);
        const UINT offset = 0;
        // Set vertex buffer to use for drawing
        context.immediateContext_->IASetVertexBuffers(0, 1, &vertexBuffer_, &stride, &offset);
        // Set index buffer of 16 byte unsigned integers to use for drawing
        context.immediateContext_->IASetIndexBuffer(indexBuffer_, DXGI_FORMAT_R16_UINT, 0);
        // Set topology - how should we interpret the indices given - to a list of triangles
        context.immediateContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        // Draw geometry in vertex buffer using index buffer
        context.immediateContext_->DrawIndexed(indexCount_, 0, 0);
        ex::endEvent(context.perf_);
    }

    /**
     * \brief Returns layout of vertices for this drawable object. To be used in shader creation.
     */
    virtual Layouts::VertexLayout_t getVertexLayout() const = 0;

protected:
    ID3D11Buffer* vertexBuffer_{ nullptr };
    ID3D11Buffer* indexBuffer_{ nullptr };
    UINT indexCount_{ 0 };

    virtual const wchar_t* getObjectName() const = 0;

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
            assert(!"Failed to create vertex buffer");
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
            assert(!"Failed to create index buffer");
            return;
        }

        // Save index count for drawing
        indexCount_ = static_cast<UINT>(indices.size());
    }
};
