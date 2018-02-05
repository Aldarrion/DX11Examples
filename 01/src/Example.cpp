#include "Example.h"
#include <iostream>

using namespace DirectX;

int Example::run(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    AllocConsole();
    FILE *pCin, *pCout, *pCerr;
    freopen_s(&pCin, "conin$", "r", stdin);
    freopen_s(&pCout, "conout$", "w", stdout);
    freopen_s(&pCerr, "conout$", "w", stderr);
    std::cout << "Start" << std::endl;
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    auto hr = context_.Init(hInstance, nCmdShow);
    if (FAILED(hr)) {
        MessageBox(nullptr, L"Failed to init context wrapper", L"Error", MB_OK);
        return 0;
    }

    hr = setup();
    if (FAILED(hr)) {
        MessageBox(nullptr, L"Failed to init device", L"Error", MB_OK);
        return 0;
    }

    mouse_ = std::make_unique<Mouse>();
    mouse_->SetWindow(context_.g_hWnd);
    mouse_->SetMode(Mouse::MODE_RELATIVE);

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
