#include "PlaneTangentBitangent.h"

PlaneTangentBitangent::PlaneTangentBitangent(ID3D11Device* device) {
    using namespace DirectX;

    // Data for the plane
    // Surface normal, shared by all the vertices
    const auto normal = XMFLOAT3(0.0f, 0.0f, -1.0f);

    const auto position0 = XMFLOAT3(-1.0f, 1.0f, 0.0f);
    const auto position1 = XMFLOAT3(-1.0f, -1.0f, 0.0f);
    const auto position2 = XMFLOAT3(1.0f, -1.0f, 0.0f);
    const auto position3 = XMFLOAT3(1.0f, 1.0f, 0.0f);

    const auto uv0 = XMFLOAT2(0.0f, 0.0f);
    const auto uv1 = XMFLOAT2(0.0f, 1.0f);
    const auto uv2 = XMFLOAT2(1.0f, 1.0f);
    const auto uv3 = XMFLOAT2(1.0f, 0.0f);


    /**
     * Calculate edges. The goal is to be able to create a transformation matrix from the tangent to model space.
     * To do this we need to calculate the tangent and bitangent vectors which in combination with the normal vector
     * compose the transformation matrix.
     * First we calculate tangent and bitangent vectors in model space.
     * We have points in model space position1, 2, and 3 which we know how to express in terms of tangent and bitangent.
     * (We know their UV coordinates - in the delta of uv0, uv1 the x is how much tangent we need, y is how much bitangent.)
     */
    const auto edge1Vec = XMLoadFloat3(&position1) - XMLoadFloat3(&position0);
    XMFLOAT3 edge_1;
    XMStoreFloat3(&edge_1, edge1Vec);

    const auto edge2Vec = XMLoadFloat3(&position2) - XMLoadFloat3(&position0);
    XMFLOAT3 edge_2;
    XMStoreFloat3(&edge_2, edge2Vec);

    const auto dUV_1Vec = XMLoadFloat2(&uv1) - XMLoadFloat2(&uv0);
    XMFLOAT2 dUV_1;
    XMStoreFloat2(&dUV_1, dUV_1Vec);

    const auto dUV_2Vec = XMLoadFloat2(&uv2) - XMLoadFloat2(&uv0);
    XMFLOAT2 dUV_2;
    XMStoreFloat2(&dUV_2, dUV_2Vec);

    /**
     * From here we just use a little bit of linear algebra to calculate the tangent and bitangent.
     * E_1 = dUV_1.x * T + dUV_1.y * B
     * E_2 = dUV_2.x * T + dUV_2.y * B
     * 
     * Using a matrix notation we get:
     * 
     * | E_1.x, E_1.y, E_1.z |  _  | dU_1, dV_1 |   | T.x, T.y, T.z |
     * | E_2.x, E_2.y, E_2.z |  ‾  | dU_2, dV_2 | * | B.x, B.y, B.z |
     * 
     * By rewriting the matrices a bit we can get the following equations.
     * For more details see https://learnopengl.com/Advanced-Lighting/Normal-Mapping
     */
    const auto constant = 1.0f / (dUV_1.x * dUV_2.y - dUV_2.x * dUV_1.y);

    // Tangent
    XMFLOAT3 tangent{
        constant * (dUV_2.y * edge_1.x - dUV_1.y * edge_2.x),
        constant * (dUV_2.y * edge_1.y - dUV_1.y * edge_2.y),
        constant * (dUV_2.y * edge_1.z - dUV_1.y * edge_2.z)
    };

    auto tangent1Vec = XMLoadFloat3(&tangent);
    tangent1Vec = XMVector3Normalize(tangent1Vec);
    XMStoreFloat3(&tangent, tangent1Vec);

    // Bitangent
    XMFLOAT3 bitangent{
        constant * (-dUV_2.x * edge_1.x + dUV_1.x * edge_2.x),
        constant * (-dUV_2.x * edge_1.y + dUV_1.x * edge_2.y),
        constant * (-dUV_2.x * edge_1.z + dUV_1.x * edge_2.z)
    };

    auto bitangent1Vec = XMLoadFloat3(&bitangent);
    bitangent1Vec = XMVector3Normalize(bitangent1Vec);
    XMStoreFloat3(&bitangent, bitangent1Vec);

    // Set calculated data to vertices
    const std::vector<VertexTypes::FullVertexTangentBitangent> vertices = {
        {position0, normal, XMFLOAT3(1.0f, 1.0f, 1.0f), uv0, tangent, bitangent},
        {position1, normal, XMFLOAT3(1.0f, 1.0f, 1.0f), uv1, tangent, bitangent},
        {position2, normal, XMFLOAT3(1.0f, 1.0f, 1.0f), uv2, tangent, bitangent},
        {position3, normal, XMFLOAT3(1.0f, 1.0f, 1.0f), uv3, tangent, bitangent}
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
