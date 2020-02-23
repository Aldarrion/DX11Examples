#pragma once
#include "ResourceHolder.h"

#include "DX11Forward.h"

class TextureSampler : public ResourceHolder {
public:
    virtual ~TextureSampler();
    
    TextureSampler(TextureSampler&& other) noexcept;
    TextureSampler& operator=(TextureSampler&& other) noexcept;

    void use(ID3D11DeviceContext* context, const UINT slot) const;

protected:
    ID3D11SamplerState* sampler_{ nullptr };
    
    TextureSampler() = default;
    static ID3D11SamplerState* createSampler(ID3D11Device* device, const D3D11_SAMPLER_DESC& sampDesc);
};
