#include "Example.h"
#include "Logging.h"

using namespace DirectX;

XMVECTORF32 Util::srgbToLinear(const XMVECTORF32& color) {
    XMVECTORF32 clearColor;
    clearColor.v = XMColorSRGBToRGB(color);
    return clearColor;
}

DirectX::XMFLOAT4 Util::srgbToLinearVec(const DirectX::XMVECTORF32& color) {
    XMVECTORF32 clearColor;
    clearColor.v = XMColorSRGBToRGB(color);
    XMFLOAT4 vec(clearColor.f[0], clearColor.f[1], clearColor.f[2], clearColor.f[3]);
    return vec;
}

ContextSettings Example::getSettings() const {
    return ContextSettings{};
}

Mouse::Mode Example::getInitialMouseMode() {
    return Mouse::MODE_RELATIVE;
}

int Example::run(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    ex::log(ex::LogLevel::Info, "Start");
    const ContextSettings settings = getSettings();

    auto hr = context_.init(hInstance, nCmdShow, settings);
    if (FAILED(hr)) {
        ex::log(ex::LogLevel::Error, "Failed to init context wrapper");
        assert(!"Failed to init context wrapper");
        return -1;
    }

    mouse_ = std::make_unique<Mouse>();
    mouse_->SetWindow(context_.hWnd_);
    mouse_->SetMode(getInitialMouseMode());


    hr = setup();
    if (FAILED(hr)) {
        ex::log(ex::LogLevel::Error, "Failed to setup the example, error: %d", hr);
        assert(!"Failed to setup the example, see the log for more info");
        return -1;
    }


    // Main message loop
    MSG msg = { 0 };
    while (WM_QUIT != msg.message && !shouldExit_) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE | PM_NOYIELD)) {
            switch (msg.message) {
            case WM_INPUT:
            case WM_MOUSEMOVE:
            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP:
            case WM_MBUTTONDOWN:
            case WM_MBUTTONUP:
            case WM_MOUSEWHEEL:
            case WM_XBUTTONDOWN:
            case WM_XBUTTONUP:
            case WM_MOUSEHOVER:
            case WM_ACTIVATEAPP:
                Mouse::ProcessMessage(msg.message, msg.wParam, msg.lParam);
                break;
            default:
                break;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            render();
        }
    }

    return static_cast<int>(msg.wParam);
}

