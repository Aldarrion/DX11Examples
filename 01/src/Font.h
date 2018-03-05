#pragma once
#include <string>
#include "PointWrapSampler.h"
#include "Texture.h"
#include "Exceptions.h"

namespace Text {
class Font {
    Texture fontMap_;
    PointWrapSampler sampler_;
    std::string glyphs_;
    int xCount_;
    int yCount_;
    int glyphPxWidth_;
    int glyphPxHeight_;
    int texturePxWidth_;
    int texturePxHeight_;
    float sizeScale_;

public:
    Font(
        ID3D11Device* device, ID3D11DeviceContext* context,
        const std::wstring& fontMap,
        const std::string& glyphs, 
        int xCount, int yCount, 
        int glyphPxWidth, int glyphPxHeight, 
        int texturePxWidth, int texturePxHeight
    )
            : fontMap_(device, context, fontMap.c_str())
            , sampler_(device)
            , glyphs_(glyphs)
            , xCount_(xCount)
            , yCount_(yCount)
            , glyphPxWidth_(glyphPxWidth) 
            , glyphPxHeight_(glyphPxHeight) 
            , texturePxWidth_(texturePxWidth)
            , texturePxHeight_(texturePxHeight)
            , sizeScale_(0.005f) {
    }

    void use(ID3D11DeviceContext* context) const {
        fontMap_.use(context, 0);
        sampler_.use(context, 0);
    }

    float getWidthSizeScale() const {
        return sizeScale_ * getFontAspectRatio();
    }
    
    float getHeightSizeScale() const {
        return sizeScale_;
    }

    float getFontAspectRatio() const {
        return glyphPxWidth_ / static_cast<float>(glyphPxHeight_);
    }

    DirectX::XMFLOAT4 getUVWH(char c) const {
        size_t idx = glyphs_.find(c);

        if (idx == std::string::npos) {
            throw Exception::InvalidCharacterException(c);
        }

        size_t y = idx / xCount_;
        size_t x = idx % xCount_;

        return { 
            x * glyphPxWidth_ / static_cast<float>(texturePxWidth_), 
            y * glyphPxHeight_ / static_cast<float>(texturePxHeight_),
            glyphPxWidth_ / static_cast<float>(texturePxWidth_),
            glyphPxHeight_ / static_cast<float>(texturePxHeight_)
        };
    }
};


// ======================
// Font factory functions
// ======================
// TODO load font properties from config file
inline Font makeInconsolata(ID3D11Device* device, ID3D11DeviceContext* context) {
    std::string glyphs = " !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
    return Font(device, context, L"textures/Inconsolata-10-bold.dds", glyphs, 16, 6, 7, 17, 112, 112);
}
}
