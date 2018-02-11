#pragma once
#include "Example.h"
#include "Camera.h"

class BasicExample : public Example {
protected:
    Camera camera_;
    DirectX::XMMATRIX projection_;

    double deltaTime_ = 0.0;
    double timeFromStart = 0.0;

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
    BasicExample() : camera_(DirectX::XMFLOAT3(0.0f, 0.0f, -10.0f)) {
    }
    virtual ~BasicExample() = default;
};
