#include "Texture.h"

#include "ContextWrapper.h"
#include "DDSTextureLoader.h"

#include <cassert>

Texture::Texture(ID3D11Device* device, ID3D11DeviceContext* context, const WCHAR* pathToDDS, bool isSRGB) {
    auto hr = DirectX::CreateDDSTextureFromFile(device, context, pathToDDS, isSRGB, &textureResource_, &texture_);
    if (FAILED(hr)) {
        assert(!"Texture could not have been loaded");
    }
}

Texture::~Texture() {
    if (texture_) 
        texture_->Release();
    if (textureResource_) 
        textureResource_->Release();
}

Texture::Texture(Texture&& other) noexcept {
    texture_ = other.texture_;
    other.texture_ = nullptr;

    textureResource_ = other.textureResource_;
    other.textureResource_ = nullptr;
}

Texture& Texture::operator=(Texture&& other) noexcept {
    std::swap(texture_, other.texture_);
    std::swap(textureResource_, other.textureResource_);

    return *this;
}

void Texture::use(ID3D11DeviceContext* context, const UINT slot) const {
    context->PSSetShaderResources(slot, 1, &texture_);
}

namespace Textures {
PTexture createSeaFloorTexture(const ContextWrapper& context) {
    return std::make_unique<Texture>(context.d3dDevice_, context.immediateContext_, L"textures/seafloor.dds", true);
}

PTexture createBoxDiffuse(const ContextWrapper& context) {
    return std::make_unique<Texture>(context.d3dDevice_, context.immediateContext_, L"textures/container2.dds", true);
}

PTexture createBoxSpecular(const ContextWrapper& context) {
    return std::make_unique<Texture>(context.d3dDevice_, context.immediateContext_, L"textures/container2_specular.dds", false);
}
}
