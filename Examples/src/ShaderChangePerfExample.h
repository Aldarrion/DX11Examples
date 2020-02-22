#pragma once

#include "BaseExample.h"
#include "WinKeyMap.h"
#include "Transform.h"
#include "FontSDF.h"

#include <memory>
#include <vector>

class Triangle;
template<typename ... TCbuffers>
class ShaderProgram;
namespace Text {
class TextSDF;
}

namespace ShaderChangePerf {

struct S1Cbuffer {
    DirectX::XMMATRIX World;
    DirectX::XMMATRIX View;
    DirectX::XMMATRIX Projection;
};

struct S2Cbuffer {
    DirectX::XMMATRIX World;
    DirectX::XMMATRIX View;
    DirectX::XMMATRIX Projection;
};

class ShaderChangePerfExample : public BaseExample {
protected:
    HRESULT setup() override;
    bool reloadShadersInternal() override;
    void handleInput() override;
    void render() override;

private:
    using Shader1 = ShaderProgram<S1Cbuffer>;
    using Shader2 = ShaderProgram<S2Cbuffer>;

    static constexpr int MODEL_COUNT = 512;

    std::unique_ptr<Shader1> shader1_;
    std::unique_ptr<Shader2> shader2_;

    std::unique_ptr<Triangle> triangle_;

    Text::FontSDF font_;
    std::unique_ptr<Text::TextSDF> frameTimeText_;

    WinKeyMap::WinKeyMap toggle2Shaders_ = WinKeyMap::E;
    bool is2ShadersOn_ = true;

    std::vector<Transform> modelTransforms_;
};

}
