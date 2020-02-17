#pragma once
#include "Example.h"
#include "Triangle.h"
#include "ShaderProgram.h"

class BasicTriangleExample : public Example {
protected:
    using SimpleShader = ShaderProgram<>;
    
    // In examples we usually use unique pointers
    // This allows us to postpone the construction of these objects
    // until we have all the data necessary.
    // It is also cleaner than to member initialize all objects from the constructor.
    std::unique_ptr<Triangle> triangle_;
    std::unique_ptr<SimpleShader> shader_;

    HRESULT setup() override;
    void render() override;

public:
    virtual ~BasicTriangleExample() = default;
};
