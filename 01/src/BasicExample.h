#pragma once
#include "Example.h"
#include "Camera.h"

class BasicExample : public Example {
protected:
    Camera camera_;

    double deltaTime = 0.0;
    double timeFromStart = 0.0;

    void handleInput(float deltaTime);
    HRESULT setup() override {
        return S_OK;
    }
    void render() override;

public:
    BasicExample() : camera_(DirectX::XMFLOAT3(0.0f, 4.0f, -10.0f)) {
    }
    virtual ~BasicExample() = default;
};
