#pragma once
#include <d3d11.h>
#include "ResourceHolder.h"

class TextureSampler : public ResourceHolder {
protected:
    ID3D11SamplerState * sampler_ = nullptr;
    
    TextureSampler() = default;

    static ID3D11SamplerState* createSampler(ID3D11Device* device, const D3D11_SAMPLER_DESC& sampDesc) {
        ID3D11SamplerState* sampler;
        auto hr = device->CreateSamplerState(&sampDesc, &sampler);
        if (FAILED(hr)) {
            std::cout << "ERROR: Could not create shadow sampler " << hr << std::endl;
            return nullptr;
        }

        return sampler;
    }

public:
    virtual ~TextureSampler() {
        if (sampler_) sampler_->Release();
    }
    
    TextureSampler(TextureSampler && other) noexcept {
        sampler_ = other.sampler_;
        other.sampler_ = nullptr;
    }

    TextureSampler& operator=(TextureSampler&& other) noexcept {
        sampler_ = other.sampler_;
        other.sampler_ = nullptr;
        
        return *this;
    }

    void use(ID3D11DeviceContext* context, const UINT slot) const {
        context->PSSetSamplers(slot, 1, &sampler_);
    }
};
