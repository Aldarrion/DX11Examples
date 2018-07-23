#include "PlaneTangentBitangent.h"

PlaneTangentBitangent::PlaneTangentBitangent(ID3D11Device* device) {
    using namespace DirectX;

    const auto normal = XMFLOAT3(0.0f, 0.0f, -1.0f);

    const auto pos1 = XMFLOAT3(-1.0f, 1.0f, 0.0f);
    const auto pos2 = XMFLOAT3(-1.0f, -1.0f, 0.0f);
    const auto pos3 = XMFLOAT3(1.0f, -1.0f, 0.0f);
    const auto pos4 = XMFLOAT3(1.0f, 1.0f, 0.0f);

    const auto uv1 = XMFLOAT2(0.0f, 0.0f);
    const auto uv2 = XMFLOAT2(0.0f, 1.0f);
    const auto uv3 = XMFLOAT2(1.0f, 1.0f);
    const auto uv4 = XMFLOAT2(1.0f, 0.0f);


    // ==========
    // Triangle 1
    // ==========
    const auto edge1Vec = XMLoadFloat3(&pos2) - XMLoadFloat3(&pos1);
    XMFLOAT3 edge1;
    XMStoreFloat3(&edge1, edge1Vec);

    const auto edge2Vec = XMLoadFloat3(&pos3) - XMLoadFloat3(&pos1);
    XMFLOAT3 edge2;
    XMStoreFloat3(&edge2, edge2Vec);

    const auto deltaUV1Vec = XMLoadFloat2(&uv2) - XMLoadFloat2(&uv1);
    XMFLOAT2 deltaUV1;
    XMStoreFloat2(&deltaUV1, deltaUV1Vec);

    const auto deltaUV2Vec = XMLoadFloat2(&uv3) - XMLoadFloat2(&uv1);
    XMFLOAT2 deltaUV2;
    XMStoreFloat2(&deltaUV2, deltaUV2Vec);

    auto f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

    // Tangent 1
    XMFLOAT3 tangent{
        f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x),
        f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y),
        f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z)
    };

    auto tangent1Vec = XMLoadFloat3(&tangent);
    tangent1Vec = XMVector3Normalize(tangent1Vec);
    XMStoreFloat3(&tangent, tangent1Vec);

    // Bitangent 1
    XMFLOAT3 bitangent{
        f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x),
        f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y),
        f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z)
    };

    auto bitangent1Vec = XMLoadFloat3(&bitangent);
    bitangent1Vec = XMVector3Normalize(bitangent1Vec);
    XMStoreFloat3(&bitangent, bitangent1Vec);


    // ==========
    // Triangle 2
    // ==========
    const auto edge3Vec = XMLoadFloat3(&pos3) - XMLoadFloat3(&pos1);
    XMFLOAT3 edge3;
    XMStoreFloat3(&edge3, edge3Vec);

    const auto edge4Vec = XMLoadFloat3(&pos4) - XMLoadFloat3(&pos1);
    XMFLOAT3 edge4;
    XMStoreFloat3(&edge4, edge4Vec);

    const auto deltaUV3Vec = XMLoadFloat2(&uv3) - XMLoadFloat2(&uv1);
    XMFLOAT2 deltaUV3;
    XMStoreFloat2(&deltaUV3, deltaUV3Vec);

    const auto deltaUV4Vec = XMLoadFloat2(&uv4) - XMLoadFloat2(&uv1);
    XMFLOAT2 deltaUV4;
    XMStoreFloat2(&deltaUV4, deltaUV4Vec);

    f = 1.0f / (deltaUV3.x * deltaUV4.y - deltaUV4.x * deltaUV3.y);

    // Tangent 2
    XMFLOAT3 tangent2{
        f * (deltaUV4.y * edge1.x - deltaUV3.y * edge2.x),
        f * (deltaUV4.y * edge1.y - deltaUV3.y * edge2.y),
        f * (deltaUV4.y * edge1.z - deltaUV3.y * edge2.z)
    };
    auto tangent2Vec = XMLoadFloat3(&tangent2);
    tangent2Vec = XMVector3Normalize(tangent2Vec);
    XMStoreFloat3(&tangent2, tangent2Vec);

    // Bitangent 2
    XMFLOAT3 bitangent2{
        f * (-deltaUV4.x * edge1.x + deltaUV3.x * edge2.x),
        f * (-deltaUV4.x * edge1.y + deltaUV3.x * edge2.y),
        f * (-deltaUV4.x * edge1.z + deltaUV3.x * edge2.z)
    };
    auto bitangent2Vec = XMLoadFloat3(&bitangent2);
    bitangent2Vec = XMVector3Normalize(bitangent2Vec);
    XMStoreFloat3(&bitangent2, bitangent2Vec);

    const std::vector<VertexTypes::FullVertexTangentBitangent> vertices = {
        {pos1, normal, XMFLOAT3(1.0f, 1.0f, 1.0f), uv1, tangent, bitangent},
        {pos2, normal, XMFLOAT3(1.0f, 1.0f, 1.0f), uv2, tangent, bitangent},
        {pos3, normal, XMFLOAT3(1.0f, 1.0f, 1.0f), uv3, tangent, bitangent},
        {pos4, normal, XMFLOAT3(1.0f, 1.0f, 1.0f), uv4, tangent, bitangent}
    };

    const std::vector<WORD> indices = {
        0, 2, 1,
        0, 3, 2
    };

    initialize(device, vertices, indices);
}

Layouts::VertexLayout_t PlaneTangentBitangent::getVertexLayout() const {
    return Layouts::TEXTURED_LAYOUT_TANGENT_BITANGENT;
}
