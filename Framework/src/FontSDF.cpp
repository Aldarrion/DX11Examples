#include "FontSDF.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#pragma warning(push)
#pragma warning(disable: 4100)
#pragma warning(disable: 4505)
#include "stb/stb_image.h"
#pragma warning(pop)

#include <fstream>

using namespace DirectX;

namespace Text {

XMFLOAT2 pxToScreen(const ContextWrapper& context, XMFLOAT2 pixels) {
    return XMFLOAT2(
        pixels.x * 1.0f / context.WIDTH,
        pixels.y * 1.0f / context.HEIGHT
    );
}

XMFLOAT2 pxToPos(const ContextWrapper& context, XMFLOAT2 pixelPos) {
    XMFLOAT2 screen(pxToScreen(context, pixelPos));
    return XMFLOAT2(
        -1.0f + screen.x * 2,
        1.0f - screen.y * 2
    );
}

FontSDF::~FontSDF() {
    if (texture_) texture_->Release();
    if (textureResource_) textureResource_->Release();
}

HRESULT FontSDF::load(const ContextWrapper& context, const std::string& fontPath) {
    sampler_ = std::make_unique<Sampler_t>(context.d3dDevice_);
    quad_ = std::make_unique<Quad>(context.d3dDevice_);

    // ---------------
    // Load font texture
    int x, y, n;
    unsigned char *data = stbi_load(std::string(fontPath + ".png").c_str(), &x, &y, &n, 4);

    if (!data)
        return S_FALSE;

    D3D11_SUBRESOURCE_DATA subResourceData{};
    subResourceData.pSysMem = data;
    subResourceData.SysMemPitch = x * 4;

    D3D11_TEXTURE2D_DESC texDesc{};
    texDesc.Width = x;
    texDesc.Height = y;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;

    auto result = context.d3dDevice_->CreateTexture2D(&texDesc, &subResourceData, &textureResource_);
    if (FAILED(result))
        return result;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;

    result = context.d3dDevice_->CreateShaderResourceView(textureResource_, &srvDesc, &texture_);
    if (FAILED(result))
        return result;


    stbi_image_free(data);


    // ---------------
    // Load font description
    std::string descriptionPath(fontPath + ".hsf");

    std::ifstream descFile(descriptionPath);

    descFile >> glyphSize_;

    int width, height;
    descFile >> width >> height;
    uvPerGlyph_.x = 1.0f / width;
    uvPerGlyph_.y = 1.0f / height;

    int glyphCount;
    descFile >> glyphCount;

    for (int i = 0; i < glyphCount; ++i) {
        int glyph;
        XMFLOAT2 coords;
        descFile >> glyph >> coords.x >> coords.y;
        charCoords_[static_cast<char>(glyph)] = coords;
    }

    return S_OK;
}

bool FontSDF::reloadShaders(ID3D11Device* device) {
    return Shaders::makeShader<SDFFontShader_t>(
        sdfShader_,
        device,
        L"shaders/SDFFont.fx", "VS",
        L"shaders/SDFFont.fx", "PS",
        quad_->getVertexLayout()
        );
}

XMFLOAT4 FontSDF::getUV(char c) const {
    auto coords = charCoords_.find(c);
    if (coords == charCoords_.end()) {
        return XMFLOAT4(0, 0, uvPerGlyph_.x, uvPerGlyph_.y);
    }

    return XMFLOAT4(
        coords->second.x * uvPerGlyph_.x,
        coords->second.y * uvPerGlyph_.y,
        uvPerGlyph_.x,
        uvPerGlyph_.y
    );
}

void FontSDF::render(const ContextWrapper& context, const std::string& text, DirectX::XMFLOAT2 position, float size) {
    context.enableBlending();
    context.disableDepthTest();

    const float aspectRatio = context.getAspectRatio();
    const float fontSize = size;
    const float scale = fontSize / context.HEIGHT;
    XMMATRIX aspectCorrection = XMMatrixScalingFromVector(XMVectorSet(scale / aspectRatio, scale, scale, 0));

    sdfShader_->use(context.immediateContext_);
    context.immediateContext_->PSSetShaderResources(0, 1, &texture_);
    sampler_->use(context.immediateContext_, 0);

    XMFLOAT2 posScreen(pxToPos(context, position));

    SDFCbuffer cb{};
    int i = 0;
    for (auto c : text) {
        if (c == '\n') {
            posScreen.y -= pxToScreen(context, XMFLOAT2(fontSize, fontSize)).y * 1.5f;
            i = 0;
            continue;
        }

        XMMATRIX move = XMMatrixTranslation(posScreen.x + pxToScreen(context, XMFLOAT2(fontSize, fontSize)).x * i, posScreen.y, 0);
        cb.Model = XMMatrixTranspose(aspectCorrection * move);
        cb.UVMul = getUV(c);
        sdfShader_->updateConstantBuffer(context.immediateContext_, cb);

        quad_->draw(context.immediateContext_);
        ++i;
    }
}

}
