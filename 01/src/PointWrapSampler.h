#pragma once
#include "TextureSampler.h"

#include <memory>

class PointWrapSampler : public TextureSampler {
public:
    explicit PointWrapSampler(ID3D11Device* device) {
        D3D11_SAMPLER_DESC sampDesc;
        ZeroMemory(&sampDesc, sizeof(sampDesc));
        sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        sampDesc.MinLOD = 0;
        sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

        sampler_ = createSampler(device, sampDesc);
    }
};

namespace Samplers {
using PPointWrapSampler = std::unique_ptr<PointWrapSampler>;

inline PPointWrapSampler createPointWrapSampler(const ContextWrapper& context) {
    return std::make_unique<PointWrapSampler>(context.d3dDevice_);
}
}
