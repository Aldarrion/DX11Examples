#pragma once
#include "Example.h"
#include "Camera.h"
#include <directxcolors.h>
#include <vector>

class BaseExample : public Example {
protected:
    Camera camera_;
    DirectX::XMMATRIX projection_;

    float deltaTime_ = 0.0f;
    float timeFromStart = 0.0f;
    static constexpr float smaPeriod_ = 30;
    float deltaTimeSMA_ = 0.0f;
    size_t frameCount_ = 0;

    virtual void handleInput();
    HRESULT setup() override {
        projection_ = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, context_.WIDTH / static_cast<FLOAT>(context_.HEIGHT), 0.01f, 100.0f);
        return S_OK;
    }
    void render() override;

    static DirectX::XMMATRIX computeNormalMatrix(const DirectX::XMMATRIX& model) {
        return XMMatrixTranspose(XMMatrixInverse(nullptr, model));
    }

    static DirectX::XMMATRIX computeNormalMatrix(const std::vector<DirectX::XMMATRIX>& matrices) {
        DirectX::XMMATRIX multiple = DirectX::XMMatrixIdentity();
        for(const auto& matrix : matrices) {
            multiple = XMMatrixMultiply(multiple, XMMatrixTranspose(matrix));
        }
        return XMMatrixTranspose(XMMatrixInverse(nullptr, multiple));
    }

    void clearViews() const {
        context_.immediateContext_->ClearRenderTargetView(context_.renderTargetView_, Util::srgbToLinear(DirectX::Colors::MidnightBlue));
        context_.immediateContext_->ClearDepthStencilView(context_.depthStencilView_, D3D11_CLEAR_DEPTH, 1.0f, 0);
    }

public:
    BaseExample() : camera_(DirectX::XMFLOAT3(0.0f, 0.0f, -10.0f)) {
    }
    virtual ~BaseExample() = default;
};
