#pragma once

#include "Quad.h"
#include "ShaderProgram.h"

#include <string>
#include <DirectXMath.h>

namespace Text {

struct SDFGlyphCb {

};

class SDFText {
private:
    Quad quad_;
    ShaderProgram<GlyphCb> fontShader_;
};

}
