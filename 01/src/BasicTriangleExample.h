#pragma once
#include "Example.h"
#include "Triangle.h"
#include "ShaderProgram.h"

class BasicTriangleExample : public Example {
protected:
    using SimpleShader = ShaderProgram<void*, false>;
    
    std::unique_ptr<Triangle> triangle_;
    std::unique_ptr<SimpleShader> shader_;

    HRESULT setup() override;
    void render() override;

public:
    virtual ~BasicTriangleExample() = default;
};
