#include "BaseExample.h"
#include <chrono>
#include <directxcolors.h>

using namespace DirectX;

void BaseExample::handleInput() {
    if (GetActiveWindow() != context_.hWnd_)
        return;

    if (GetAsyncKeyState(VK_ESCAPE)) {
        shouldExit_ = true;
    }
    if (GetAsyncKeyState(0x57)) { // W
        camera_.ProcessKeyboard(CameraMovement::FORWARD, deltaTime_);
    }
    if (GetAsyncKeyState(0x53)) { // S
        camera_.ProcessKeyboard(CameraMovement::BACKWARD, deltaTime_);
    }
    if (GetAsyncKeyState(0x41)) { // A
        camera_.ProcessKeyboard(CameraMovement::LEFT, deltaTime_);
    }
    if (GetAsyncKeyState(0x44)) { // D
        camera_.ProcessKeyboard(CameraMovement::RIGHT, deltaTime_);
    }
    if (GetAsyncKeyState(0x20)) { // Space
        camera_.ProcessKeyboard(CameraMovement::UP, deltaTime_);
    }
    if (GetAsyncKeyState(0x11)) { // Ctrl
        camera_.ProcessKeyboard(CameraMovement::DOWN, deltaTime_);
    }

	auto mouse = mouse_->GetState();
	if (GetAsyncKeyState(0x4D) & 1) { // M
		if (mouse.positionMode == Mouse::MODE_RELATIVE)	{
			mouse_->SetMode(Mouse::MODE_ABSOLUTE);
		} else {
			mouse_->SetMode(Mouse::MODE_RELATIVE);
		}
	}
    
    if (mouse.positionMode == Mouse::MODE_RELATIVE) {
        camera_.ProcessMouseMovement(static_cast<float>(-mouse.x), static_cast<float>(mouse.y));
    }
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
