#pragma once
#include "BaseExample.h"
#include "ShaderProgram.h"
#include "ColorCube.h"
#include "PhongLights.h"
#include "ConstantBuffers.h"

class PhongShadingExample : public BaseExample {
protected:
    std::unique_ptr<ShaderProgram<ConstantBuffers::PhongCB>> cubeShader_;
    std::unique_ptr<ShaderProgram<ConstantBuffers::SolidConstBuffer>> solidShader_;
    std::unique_ptr<ColorCube> colorCube_;

    HRESULT setup() override;
    void render() override;

public:
    virtual ~PhongShadingExample() = default;
};
