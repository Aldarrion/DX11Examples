#pragma once
#include "BaseExample.h"
#include "ShaderProgram.h"
#include "TexturedCube.h"
#include "Texture.h"
#include "AnisotropicSampler.h"
#include "Plane.h"
#include "TextSDF.h"

#include <wrl/client.h>

namespace Billboard {

constexpr size_t GRASS_COUNT = 1000;

struct BillboardCBuffer {
    DirectX::XMMATRIX World;
    DirectX::XMMATRIX View;
    DirectX::XMMATRIX Projection;
    DirectX::XMMATRIX NormalMatrix;
    DirectX::XMMATRIX GrassModels[3];
    DirectX::XMMATRIX GrassMatrices[GRASS_COUNT];
    int IsInstanced;
};

class BillboardExample : public BaseExample {
protected:
    bool isInstanced_ = true;

    using BillboardShader = ShaderProgram<BillboardCBuffer>;
    using TextureShader = ShaderProgram<BillboardCBuffer>;

    std::unique_ptr<BillboardShader> billboardShader_;
    std::unique_ptr<Plane> plane_;
    std::unique_ptr<TextureShader> textureShader_;
    std::unique_ptr<Texture> seaFloorTexture_;
    std::unique_ptr<Texture> grassBillboard_;
    std::unique_ptr<AnisotropicSampler> diffuseSampler_;
    std::vector<DirectX::XMMATRIX> grassPositions_;
    Text::FontSDF font_;
    std::unique_ptr<Text::TextSDF> frameTimeText_;

    // If we don't want to handle 
    Microsoft::WRL::ComPtr<ID3D11BlendState> blendState_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer_;

    HRESULT setup() override;
    bool reloadShadersInternal() override;
    void handleInput() override;
    void render() override;
};
}
