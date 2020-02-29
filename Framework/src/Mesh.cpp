#include "Mesh.h"

#include "Logging.h"

#define COM_NO_WINDOWS_H
#include <d3d11_1.h>

namespace Models {

Mesh::Mesh(
    ID3D11Device* device, 
    const std::vector<Vertex>& vertices, 
    const std::vector<unsigned int>& indices, 
    std::vector<ModelTexture>&& textures)
        : textures_(std::move(textures)) {
    
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = static_cast<UINT>(sizeof(Vertex) * vertices.size());
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices.data();
    auto hr = device->CreateBuffer(&bd, &InitData, &vertexBuffer_);
    if (FAILED(hr)) {
        assert(!"Failed to create vertex buffer");
        return;
    }

    // Create index buffer
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = static_cast<UINT>(sizeof(unsigned int) * indices.size());
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    InitData.pSysMem = indices.data();
    hr = device->CreateBuffer(&bd, &InitData, &indexBuffer_);
    if (FAILED(hr)) {
        assert(!"Failed to create index buffer");
        return;
    }
    indexCount_ = static_cast<UINT>(indices.size());
}

Mesh::~Mesh() {
    if (vertexBuffer_) vertexBuffer_->Release();
    if (indexBuffer_) indexBuffer_->Release();
}

Mesh::Mesh(Mesh&& other) noexcept {
    vertexBuffer_ = other.vertexBuffer_;
    other.vertexBuffer_ = nullptr;
    indexBuffer_ = other.indexBuffer_;
    other.indexBuffer_ = nullptr;
    indexCount_ = other.indexCount_;
    other.indexCount_ = 0;
    textures_ = std::move(other.textures_);
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
    vertexBuffer_ = other.vertexBuffer_;
    other.vertexBuffer_ = nullptr;
    indexBuffer_ = other.indexBuffer_;
    other.indexBuffer_ = nullptr;
    indexCount_ = other.indexCount_;
    other.indexCount_ = 0;
    textures_ = std::move(other.textures_);

    return *this;
}

void Mesh::draw(ID3D11DeviceContext* context) const {
    for (UINT i = 0; i < textures_.size(); ++i) {
        if (textures_[i].Type != TextureType::Unknown) {
            textures_[i].Texture->use(context, static_cast<int>(textures_[i].Type));
        } else {
            ex::log(ex::LogLevel::Error, "Trying to use texture of unknown type");
        }
    }

    const UINT stride = sizeof(Vertex);
    const UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &vertexBuffer_, &stride, &offset);
    context->IASetIndexBuffer(indexBuffer_, DXGI_FORMAT_R32_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->DrawIndexed(indexCount_, 0, 0);
}

}
