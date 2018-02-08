#pragma once
#include <d3d11.h>
#include "DDSTextureLoader.h"

class Texture {
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

    Texture(const Texture&) = delete;
    Texture operator=(const Texture&) = delete;

    void use(ID3D11DeviceContext* context, const UINT slot) const {
        context->PSSetShaderResources(slot, 1, &texture_);
    }
};
