#pragma once
#include <d3d11.h>
#include "DDSTextureLoader.h"
#include "ResourceHolder.h"

class Texture : public ResourceHolder {
private:
    ID3D11ShaderResourceView * texture_;

public:
    Texture(ID3D11Device* device, ID3D11DeviceContext* context, const WCHAR* pathToDDS) {
        auto hr = DirectX::CreateDDSTextureFromFile(device, context, pathToDDS, nullptr, &texture_);
        if (FAILED(hr)) {
            MessageBox(nullptr, L"Texture could not have been loaded", L"Error", MB_OK);
        }
    }
    ~Texture() {
        if (texture_) texture_->Release();
    }

    void use(ID3D11DeviceContext* context, const UINT slot) const {
        context->PSSetShaderResources(slot, 1, &texture_);
    }
};

namespace Textures {
    using PTexture = std::unique_ptr<Texture>;
    
    inline PTexture createSeaFloorTexture(const ContextWrapper& context) {
        return std::make_unique<Texture>(context.d3dDevice_, context.immediateContext_, L"textures/seafloor.dds");
    }

    inline PTexture createBoxDiffuse(const ContextWrapper& context) {
        return std::make_unique<Texture>(context.d3dDevice_, context.immediateContext_, L"textures/container2.dds");
    }

    inline PTexture createBoxSpecular(const ContextWrapper& context) {
        return std::make_unique<Texture>(context.d3dDevice_, context.immediateContext_, L"textures/container2_specular.dds");
    }
}
