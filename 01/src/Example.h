#pragma once
#include "ContextWrapper.h"

#include "Mouse.h"
#include <windows.h>
#include <memory>

class Example {
protected:
    bool shouldExit_ = false;
    std::unique_ptr<DirectX::Mouse> mouse_;
    ContextWrapper context_;

    virtual HRESULT setup() = 0;
    virtual void render() = 0;

public:
    virtual ~Example() = default;
    int run(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow);
};
