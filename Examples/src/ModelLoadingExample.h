#pragma once
#include "BaseExample.h"
#include "Model.h"
#include "ShaderProgram.h"
#include "AnisotropicSampler.h"

namespace ModelLoading {

struct SpecularCB {
    DirectX::XMMATRIX World;
    DirectX::XMMATRIX View;
    DirectX::XMMATRIX Projection;
    DirectX::XMMATRIX NormalMatrix;
    DirLight SunLight;
    DirectX::XMFLOAT3 ViewPos;
    int UseSpecular;
};

class ModelLoadingExample : public BaseExample {
protected:
    std::unique_ptr<Models::Model> model_;
    Samplers::PAnisotropicSampler sampler_;
    std::unique_ptr<ShaderProgram<SpecularCB>> shader_;
    
    HRESULT setup() override;
    bool reloadShadersInternal() override;
    void render() override;
};
}
