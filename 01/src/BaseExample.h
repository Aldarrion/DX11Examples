#pragma once
#include "Example.h"
#include "Camera.h"

class BaseExample : public Example {
protected:
    Camera camera_;
    DirectX::XMMATRIX projection_;

    float deltaTime_ = 0.0f;
    float timeFromStart = 0.0f;

    void handleInput(float deltaTime);
    HRESULT setup() override {
        projection_ = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, context_.WIDTH / static_cast<FLOAT>(context_.HEIGHT), 0.01f, 100.0f);
        return S_OK;
    }
    void render() override;

    static DirectX::XMMATRIX computeNormalMatrix(const DirectX::XMMATRIX& model) {
        return XMMatrixTranspose(XMMatrixInverse(nullptr, (model)));
    }

public:
    BaseExample() : camera_(DirectX::XMFLOAT3(0.0f, 0.0f, -10.0f)) {
    }
    virtual ~BaseExample() = default;
};
