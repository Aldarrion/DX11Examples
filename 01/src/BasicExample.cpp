#include "BasicExample.h"
#include <chrono>
#include <directxcolors.h>

using namespace DirectX;

void BasicExample::handleInput(float deltaTime) {
    if (GetActiveWindow() != context_.hWnd_)
        return;

    if (GetAsyncKeyState(VK_ESCAPE)) {
        shouldExit_ = true;
    }
    if (GetAsyncKeyState(0x57)) { // W
        camera_.ProcessKeyboard(CameraMovement::FORWARD, deltaTime);
    }
    if (GetAsyncKeyState(0x53)) { // S
        camera_.ProcessKeyboard(CameraMovement::BACKWARD, deltaTime);
    }
    if (GetAsyncKeyState(0x41)) { // A
        camera_.ProcessKeyboard(CameraMovement::LEFT, deltaTime);
    }
    if (GetAsyncKeyState(0x44)) { // D
        camera_.ProcessKeyboard(CameraMovement::RIGHT, deltaTime);
    }
    if (GetAsyncKeyState(0x20)) { // Space
        camera_.ProcessKeyboard(CameraMovement::UP, deltaTime);
    }
    if (GetAsyncKeyState(0x11)) { // Ctrl
        camera_.ProcessKeyboard(CameraMovement::DOWN, deltaTime);
    }
    auto mouse = mouse_->GetState();
    if (mouse.positionMode == Mouse::MODE_RELATIVE) {
        camera_.ProcessMouseMovement(static_cast<float>(-mouse.x), static_cast<float>(mouse.y));
    }
}

std::chrono::steady_clock::time_point lastFrame = std::chrono::high_resolution_clock::now();
void BasicExample::render() {
    // Update our time
    const auto currentFrame = std::chrono::high_resolution_clock::now();
    deltaTime_ = std::chrono::duration_cast<std::chrono::nanoseconds>(currentFrame - lastFrame).count() / 1000000000.0;
    lastFrame = currentFrame;
    timeFromStart += deltaTime_;

    handleInput(static_cast<float>(deltaTime_));
}
