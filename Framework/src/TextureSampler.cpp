#include "TextureSampler.h"
#include "Logging.h"

#define COM_NO_WINDOWS_H
#include <d3d11_1.h>

TextureSampler::~TextureSampler() {
    if (sampler_)
        sampler_->Release();
}

TextureSampler::TextureSampler(TextureSampler&& other) noexcept {
    sampler_ = other.sampler_;
    other.sampler_ = nullptr;
}

TextureSampler& TextureSampler::operator=(TextureSampler&& other) noexcept {
    sampler_ = other.sampler_;
    other.sampler_ = nullptr;

    return *this;
}

void TextureSampler::use(ID3D11DeviceContext* context, const UINT slot) const {
    context->PSSetSamplers(slot, 1, &sampler_);
}

ID3D11SamplerState* TextureSampler::createSampler(ID3D11Device* device, const D3D11_SAMPLER_DESC& sampDesc) {
    ID3D11SamplerState* sampler;
    auto hr = device->CreateSamplerState(&sampDesc, &sampler);
    if (FAILED(hr)) {
        ex::log(ex::LogLevel::Error, "Could not create shadow sampler, error code: %d", hr);
        return nullptr;
    }

    return sampler;
}
