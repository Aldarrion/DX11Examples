#pragma once
#define NOMINMAX // Disable Windows min() max() macros

#include "ContextWrapper.h"

#include "Mouse.h"
#include <windows.h>
#include <memory>

/**
 * \brief Abstract base class for all examples. Creates only the very minimal update loop with a window.
 */
class Example {
protected:
    bool shouldExit_ = false;
    std::unique_ptr<DirectX::Mouse> mouse_;
    ContextWrapper context_;

    /**
     * \brief Called before first frame. Use to initialize your example.
     */
    virtual HRESULT setup() = 0;
    
    /**
     * \brief Called every frame. Use to update and render your example.
     */
    virtual void render() = 0;

public:
    virtual ~Example() = default;
    int run(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow);
};
