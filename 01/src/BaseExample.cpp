#include "BaseExample.h"
#include <chrono>
#include <directxcolors.h>
#include "WinKeyMap.h"

using namespace DirectX;

HRESULT BaseExample::reloadShaders() {
    if (reloadShadersInternal())
        return S_OK;
    return E_FAIL;
}

void BaseExample::handleInput() {
    if (GetActiveWindow() != context_.hWnd_)
        return;

    if (GetAsyncKeyState(VK_ESCAPE)) {
        shouldExit_ = true;
    }
    if (GetAsyncKeyState(WinKeyMap::W)) {
        camera_.ProcessKeyboard(CameraMovement::FORWARD, deltaTime_);
    }
    if (GetAsyncKeyState(WinKeyMap::S)) {
        camera_.ProcessKeyboard(CameraMovement::BACKWARD, deltaTime_);
    }
    if (GetAsyncKeyState(WinKeyMap::A)) {
        camera_.ProcessKeyboard(CameraMovement::LEFT, deltaTime_);
    }
    if (GetAsyncKeyState(WinKeyMap::D)) {
        camera_.ProcessKeyboard(CameraMovement::RIGHT, deltaTime_);
    }
    if (GetAsyncKeyState(VK_SPACE)) {
        camera_.ProcessKeyboard(CameraMovement::UP, deltaTime_);
    }
    if (GetAsyncKeyState(VK_CONTROL)) {
        camera_.ProcessKeyboard(CameraMovement::DOWN, deltaTime_);
    }

    auto mouseState = mouse_->GetState();
    if (GetAsyncKeyState(WinKeyMap::M) & 1) {
        if (mouseState.positionMode == Mouse::MODE_RELATIVE) {
            mouse_->SetMode(Mouse::MODE_ABSOLUTE);
        } else {
            mouse_->SetMode(Mouse::MODE_RELATIVE);
        }
    }

    if (mouseState.positionMode == Mouse::MODE_RELATIVE) {
        camera_.ProcessMouseMovement(static_cast<float>(-mouseState.x), static_cast<float>(mouseState.y));
    }

    if (GetAsyncKeyState(WinKeyMap::F5) & 1) {
        std::cout << "+++ Reloading shaders" << std::endl;
        if (FAILED(reloadShaders()))
            std::cout << "--- Reload failed" << std::endl;
        else
            std::cout << "--- Reload successful" << std::endl;
    }
}

HRESULT BaseExample::setup() {
    projection_ = DirectX::XMMatrixPerspectiveFovLH(XM_PIDIV4, context_.WIDTH / static_cast<FLOAT>(context_.HEIGHT), 0.01f, 100.0f);
    return S_OK;
}

std::chrono::steady_clock::time_point lastFrame = std::chrono::high_resolution_clock::now();
void BaseExample::render() {
    ++frameCount_;
    // Update our time
    const auto currentFrame = std::chrono::high_resolution_clock::now();
    deltaTime_ = std::chrono::duration_cast<std::chrono::nanoseconds>(currentFrame - lastFrame).count() / 1000000000.0f;
    lastFrame = currentFrame;
    timeFromStart += deltaTime_;

    // Compute frame moving average
    if (frameCount_ == smaPeriod_) {
        deltaTimeSMA_ = timeFromStart / smaPeriod_;
    } else if (frameCount_ > smaPeriod_) {
        float newSmaSum = deltaTimeSMA_ * (smaPeriod_ - 1) + deltaTime_;
        deltaTimeSMA_ = newSmaSum / smaPeriod_;
    }

    handleInput();
}

XMMATRIX BaseExample::computeNormalMatrix(const DirectX::XMMATRIX & model) {
    return XMMatrixTranspose(XMMatrixInverse(nullptr, model));
}

XMMATRIX BaseExample::computeNormalMatrix(const std::vector<DirectX::XMMATRIX>& matrices) {
    XMMATRIX multiple = DirectX::XMMatrixIdentity();
    for (const auto& matrix : matrices) {
        multiple = XMMatrixMultiply(multiple, XMMatrixTranspose(matrix));
    }
    return XMMatrixTranspose(XMMatrixInverse(nullptr, multiple));
}

void BaseExample::clearViews() const {
    context_.immediateContext_->ClearRenderTargetView(context_.renderTargetView_, Util::srgbToLinear(DirectX::Colors::MidnightBlue));
    context_.immediateContext_->ClearDepthStencilView(context_.depthStencilView_, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

BaseExample::BaseExample() 
    : camera_(XMFLOAT3(0.0f, 0.0f, -10.0f)) {
}
