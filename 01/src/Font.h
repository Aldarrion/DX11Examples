#pragma once
#include <string>


namespace Text {
class Font {
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
        const std::string& glyphs, 
        int xCount, int yCount, 
        int glyphPxWidth, int glyphPxHeight, 
        int texturePxWidth, int texturePxHeight
    )
            : glyphs_(glyphs)
            , xCount_(xCount)
            , yCount_(yCount)
            , glyphPxWidth_(glyphPxWidth) 
            , glyphPxHeight_(glyphPxHeight) 
            , texturePxWidth_(texturePxWidth)
            , texturePxHeight_(texturePxHeight)
            , sizeScale_(0.006f) {
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

inline Font makeInconsolata() {
    std::string glyphs = " !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
    return Font(glyphs, 16, 6, 7, 17, 112, 112);
}
}
