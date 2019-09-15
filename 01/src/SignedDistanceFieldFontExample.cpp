#include "SignedDistanceFieldFontExample.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb/stb_image.h"

#include <fstream>

using namespace DirectX;

namespace SDF {

SignedDistanceFieldFontExample::~SignedDistanceFieldFontExample() {
    if (texture_) texture_->Release();
    if (textureResource_) textureResource_->Release();
}

HRESULT SignedDistanceFieldFontExample::setup() {
    auto result = BaseExample::setup();

    //msdfImage_ = std::make_unique<Texture>(context_.d3dDevice_, context_.immediateContext_, L, true);
    sampler_ = std::make_unique<Sampler_t>(context_.d3dDevice_);
    quad_ = std::make_unique<Quad>(context_.d3dDevice_);
    

    int x, y, n;
    unsigned char *data = stbi_load("fonts/RobotoMono-Regular.png", &x, &y, &n, 4);

    if (!data)
        return S_FALSE;

    font_ = FontSDF("fonts/RobotoMono-Regular.hsf");

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

    result = context_.d3dDevice_->CreateTexture2D(&texDesc, &subResourceData, &textureResource_);
    if (!SUCCEEDED(result))
        return result;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;

    result = context_.d3dDevice_->CreateShaderResourceView(textureResource_, &srvDesc, &texture_);
    if (!SUCCEEDED(result))
        return result;


    stbi_image_free(data);

    context_.enableBlending();
    context_.disableDepthTest();

    reloadShaders();
    showMouse();

    return result;
}

Mouse::Mode SignedDistanceFieldFontExample::getInitialMouseMode() {
    // Make mouse visible
    return Mouse::MODE_ABSOLUTE;
}

bool SignedDistanceFieldFontExample::reloadShadersInternal() {
    return Shaders::makeShader<SDFFontShader_t>(
        sdfShader_,
        context_.d3dDevice_,
        L"shaders/SDFFont.fx", "VS",
        L"shaders/SDFFont.fx", "PS",
        quad_->getVertexLayout()
    );
}

void SignedDistanceFieldFontExample::handleInput() {
    BaseExample::handleInput();
}

XMFLOAT2 pxToScreen(XMFLOAT2 pixels) {
    return XMFLOAT2(
        pixels.x * 0.5f / 1280.0f,
        pixels.y * 0.5f / 720.0f
    );
}

XMFLOAT2 pxToPos(XMFLOAT2 pixelPos) {
    XMFLOAT2 screen(pxToScreen(pixelPos));
    return XMFLOAT2(
        -1.0f + screen.x,
        1.0f - screen.y
    );
}

void SignedDistanceFieldFontExample::render() {
    BaseExample::render();

    clearViews();

    const float aspectRatio = context_.getAspectRatio();
    //const float scale = 0.015f;
    const float fontSize = 36.0f;
    const float scale = fontSize / 1280.0f;
    XMMATRIX aspectCorrection = XMMatrixScalingFromVector({ scale / aspectRatio, scale, scale });


    SDFCbuffer cb{};

    sdfShader_->use(context_.immediateContext_);
    context_.immediateContext_->PSSetShaderResources(0, 1, &texture_);
    sampler_->use(context_.immediateContext_, 0);

    int i = 0;
    for (auto c : "Hello") {
        XMFLOAT2 screenPos = pxToPos(XMFLOAT2(2 * fontSize, 2 * fontSize));
        XMMATRIX move = XMMatrixTranslation(screenPos.x + pxToScreen(XMFLOAT2(fontSize, fontSize)).x * i, screenPos.y, 0);
        cb.Model = XMMatrixTranspose(aspectCorrection * move);
        cb.UVMul = font_.getUV(c);
        sdfShader_->updateConstantBuffer(context_.immediateContext_, cb);

        quad_->draw(context_.immediateContext_);
        ++i;
    }

    context_.swapChain_->Present(0, 0);
}


FontSDF::FontSDF(const char* descriptionPath) {
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

}