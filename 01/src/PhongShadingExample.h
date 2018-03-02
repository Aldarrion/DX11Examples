#pragma once
#include "BaseExample.h"
#include "ShaderProgram.h"
#include "ColorCube.h"
#include "PhongLights.h"
#include "ConstantBuffers.h"

namespace Phong {

class PhongShadingExample : public BaseExample {
protected:
    using PhongShader = ShaderProgram<ConstantBuffers::PhongCB>;
    using SolidShader = ShaderProgram<ConstantBuffers::SolidConstBuffer>;

    std::unique_ptr<PhongShader> phongShader_;
    std::unique_ptr<SolidShader> solidShader_;
    std::unique_ptr<ColorCube> colorCube_;

    HRESULT setup() override;
    void render() override;

public:
    virtual ~PhongShadingExample() = default;
};
}
