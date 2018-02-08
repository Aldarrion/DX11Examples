#pragma once
#include "TextureSampler.h"
#include <iostream>

class ShadowSampler : public TextureSampler {
public:
    explicit ShadowSampler(ID3D11Device* device) {
        D3D11_SAMPLER_DESC sampDesc;
        ZeroMemory(&sampDesc, sizeof(sampDesc));
        // Shadows need point sampler, filtering needs to be done afterwards.
        // Averaging depths would do no good
        sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        // If point is out of bounds of shadowmap assume it's not in shadow
        sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
        sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
        sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
        // White = no shadow
        sampDesc.BorderColor[0] = 1.0f;
        sampDesc.BorderColor[1] = 1.0f;
        sampDesc.BorderColor[2] = 1.0f;
        sampDesc.BorderColor[3] = 1.0f;
        sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        sampDesc.MinLOD = 0;
        sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
        
        sampler_ = createSampler(device, sampDesc);
    }
};
