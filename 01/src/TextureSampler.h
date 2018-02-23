#pragma once
#include <d3d11.h>

class TextureSampler {
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
    
    TextureSampler(const TextureSampler&) = delete;
    TextureSampler operator=(const TextureSampler&) = delete;

    void use(ID3D11DeviceContext* context, const UINT slot) const {
        context->PSSetSamplers(slot, 1, &sampler_);
    }
};
