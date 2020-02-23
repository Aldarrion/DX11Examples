#pragma once

#include "ResourceHolder.h"
#include "DX11Forward.h"

#include <memory>

struct ContextWrapper;

class Texture : public ResourceHolder {
public:
    Texture(ID3D11Device* device, ID3D11DeviceContext* context, const WCHAR* pathToDDS, bool isSRGB);
    ~Texture();
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    void use(ID3D11DeviceContext* context, const UINT slot) const;

private:
    ID3D11ShaderResourceView* texture_{};
    ID3D11Resource* textureResource_{};
};

namespace Textures {
    using PTexture = std::unique_ptr<Texture>;
    
    PTexture createSeaFloorTexture(const ContextWrapper& context);
    PTexture createBoxDiffuse(const ContextWrapper& context);
    PTexture createBoxSpecular(const ContextWrapper& context);
}
