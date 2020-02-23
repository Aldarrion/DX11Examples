#pragma once

#include <vector>
#include <DirectXMath.h>
#include "ModelTexture.h"

struct ID3D11Buffer;
struct ID3D11DeviceContext;
struct ID3D11Device;

namespace Models {

struct Vertex {
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT3 Normal;
    DirectX::XMFLOAT2 TexCoords;
    /*DirectX::XMFLOAT3 Tangent;
    DirectX::XMFLOAT3 Bitangent;*/
};

class Mesh {
public:
    Mesh(
        ID3D11Device* device,
        const std::vector<Vertex>& vertices,
        const std::vector<unsigned int>& indices,
        std::vector<ModelTexture>&& textures
    );

    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh operator=(const Mesh&) = delete;

    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    void draw(ID3D11DeviceContext* context) const;

private:
    ID3D11Buffer* vertexBuffer_;
    ID3D11Buffer* indexBuffer_;
    UINT indexCount_;
    std::vector<ModelTexture> textures_;
};

}
