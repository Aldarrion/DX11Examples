#pragma once
#include <DirectXMath.h>

struct Transform {
    DirectX::XMFLOAT3 Position;
    /**
     * Rotation in radians
     */
    DirectX::XMFLOAT3 RollPithYaw;
    DirectX::XMFLOAT3 Scale;

    explicit Transform(
        const DirectX::XMFLOAT3& position = DirectX::XMFLOAT3(),
        const DirectX::XMFLOAT3& rollPitchYaw = DirectX::XMFLOAT3(),
        const DirectX::XMFLOAT3& scale = DirectX::XMFLOAT3(1.0, 1.0, 1.0))
            : Position(position)
            , RollPithYaw(rollPitchYaw)
            , Scale(scale) {
    }

    DirectX::XMMATRIX generateModelMatrix() const {
        const DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&RollPithYaw));
        const DirectX::XMMATRIX scale = DirectX::XMMatrixScalingFromVector(XMLoadFloat3(&Scale));
        const DirectX::XMMATRIX traslation = DirectX::XMMatrixTranslationFromVector(XMLoadFloat3(&Position));
        return rotation * scale * traslation;
    }
};
