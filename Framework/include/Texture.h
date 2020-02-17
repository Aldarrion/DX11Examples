#pragma once
#include "DDSTextureLoader.h"
#include "ResourceHolder.h"
#include "ContextWrapper.h"

#include <memory>
#include <utility>
#include <d3d11.h>

class Texture : public ResourceHolder {
public:
    Texture(ID3D11Device* device, ID3D11DeviceContext* context, const WCHAR* pathToDDS, bool isSRGB) {
        auto hr = DirectX::CreateDDSTextureFromFile(device, context, pathToDDS, isSRGB, &textureResource_, &texture_);
        if (FAILED(hr)) {
            MessageBoxA(nullptr, "Texture could not have been loaded", "Error", MB_OK);
        }
    }
    ~Texture() {
        if (texture_) texture_->Release();
        if (textureResource_) textureResource_->Release();
    }

    Texture(Texture&& other) noexcept {
        texture_ = other.texture_;
        other.texture_ = nullptr;

        textureResource_ = other.textureResource_;
        other.textureResource_ = nullptr;
    }

    Texture& operator=(Texture&& other) noexcept {
        std::swap(texture_, other.texture_);
        std::swap(textureResource_, other.textureResource_);
        
        return *this;
    }

    void use(ID3D11DeviceContext* context, const UINT slot) const {
        context->PSSetShaderResources(slot, 1, &texture_);
    }

private:
    ID3D11ShaderResourceView* texture_{};
    ID3D11Resource* textureResource_{};
};

namespace Textures {
    using PTexture = std::unique_ptr<Texture>;
    
    inline PTexture createSeaFloorTexture(const ContextWrapper& context) {
        return std::make_unique<Texture>(context.d3dDevice_, context.immediateContext_, L"textures/seafloor.dds", true);
    }

    inline PTexture createBoxDiffuse(const ContextWrapper& context) {
        return std::make_unique<Texture>(context.d3dDevice_, context.immediateContext_, L"textures/container2.dds", true);
    }

    inline PTexture createBoxSpecular(const ContextWrapper& context) {
        return std::make_unique<Texture>(context.d3dDevice_, context.immediateContext_, L"textures/container2_specular.dds", false);
    }
}
