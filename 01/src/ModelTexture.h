#pragma once
#include "Texture.h"
#include <memory>

namespace Models {

enum class TextureType {
    Diffuse,
    Specular,
    Normal,
    Height,
    Unknown
};

struct ModelTexture {
    Textures::PTexture Texture;
    TextureType Type;
    explicit ModelTexture(Textures::PTexture&& texture, const TextureType type)
        : Texture(std::move(texture))
        , Type(type) {
    }
};
}
