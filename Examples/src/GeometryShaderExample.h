#pragma once
#include "BaseExample.h"
#include "ShaderProgram.h"
#include "Texture.h"
#include "AnisotropicSampler.h"
#include "TexturedCube.h"

namespace GeometryShader {

struct ConstantBuffer {
    DirectX::XMMATRIX World;
    DirectX::XMMATRIX View;
    DirectX::XMMATRIX Projection;
    DirectX::XMMATRIX NormalMatrix;
};

class GeometryShaderExample : public BaseExample {
protected:
    using GeomShader = ShaderProgram<ConstantBuffer>;
    using StdShader = ShaderProgram<ConstantBuffer>;

    std::unique_ptr<StdShader> standardShader_;
    std::unique_ptr<GeomShader> normalShader_;
    std::unique_ptr<Texture> texture_;
    std::unique_ptr<AnisotropicSampler> sampler_;
    std::unique_ptr<TexturedCube> cube_;
    
    HRESULT setup() override;
    bool reloadShadersInternal() override;
    void render() override;

public:
    virtual ~GeometryShaderExample() = default;
};
}
