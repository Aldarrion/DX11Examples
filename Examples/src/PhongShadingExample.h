#pragma once
#include "BaseExample.h"
#include "ShaderProgram.h"
#include "ColorCube.h"
#include "PhongLights.h"
#include "ConstantBuffers.h"
#include "TextSDF.h"

namespace Phong {

class PhongShadingExample : public BaseExample {
protected:
    using PhongShader = ShaderProgram<ConstantBuffers::PhongCB>;
    using SolidShader = ShaderProgram<ConstantBuffers::SolidConstBuffer>;

    std::unique_ptr<PhongShader> phongShader_;
    std::unique_ptr<SolidShader> solidShader_;
    std::unique_ptr<ColorCube> colorCube_;

    Text::FontSDF font_;
    std::unique_ptr<Text::TextSDF> infoText_;
    float shininess_ = 32.0f;

    HRESULT setup() override;
    bool reloadShadersInternal() override;
    void render() override;
    void handleInput() override;

public:
    virtual ~PhongShadingExample() = default;
};
}
