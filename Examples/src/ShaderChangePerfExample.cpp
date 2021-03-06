#include "ShaderChangePerfExample.h"

#include "Triangle.h"
#include "ShaderProgram.h"
#include "Layouts.h"
#include "Transform.h"
#include "TextSDF.h"
#include "Logging.h"

namespace ShaderChangePerf {
using namespace DirectX;

HRESULT ShaderChangePerfExample::setup() {
    auto hr = BaseExample::setup();
    if (FAILED(hr))
        return hr;

    Text::makeDefaultSDFFont(context_, font_);
    frameTimeText_ = std::make_unique<Text::TextSDF>("Frame time: 0", &font_);

    hr = reloadShaders();
    if (FAILED(hr))
        return hr;
    
    triangle_ = std::make_unique<Triangle>(context_.d3dDevice_);

    for (int i = 0; i < MODEL_COUNT; ++i) {
        modelTransforms_.emplace_back(XMFLOAT3(0, -2, i / 2.0f));
    }

    return S_OK;
}

bool ShaderChangePerfExample::reloadShadersInternal() {
    return 
        Shaders::makeShader<Shader1>(shader1_, context_.d3dDevice_, "shaders/ChangePerf_1.fx", "VS", "shaders/ChangePerf_1.fx", "PS", Layouts::POS_LAYOUT)
        && Shaders::makeShader<Shader2>(shader2_, context_.d3dDevice_, "shaders/ChangePerf_2.fx", "VS", "shaders/ChangePerf_2.fx", "PS", Layouts::POS_LAYOUT);
}

void ShaderChangePerfExample::handleInput() {
    BaseExample::handleInput();

    if (GetAsyncKeyState(toggle2Shaders_) & 1) {
        is2ShadersOn_ = !is2ShadersOn_;
    }
}

float timeToShowFps = 0.0f;
bool isEnd = false;
float benchmarkTime = 10.0f;

void ShaderChangePerfExample::render() {
    BaseExample::render();

    timeToShowFps += deltaTimeSMA_;
    if (timeToShowFps > 0.5f) {
        timeToShowFps = 0.0f;
        frameTimeText_->setText("\n Frame time (ms): " + std::to_string(deltaTimeSMA_ * 1000) +
            "\n E: toggle 2 shader rendering. Using 2 shaders: " + std::to_string(is2ShadersOn_)
        );
    }

    clearViews();

    frameTimeText_->draw(context_);

    bool isEven = true;
    S1Cbuffer s1cb{};
    s1cb.Projection = XMMatrixTranspose(projection_);
    s1cb.View = XMMatrixTranspose(camera_.getViewMatrix());
    S2Cbuffer s2cb{};
    if (is2ShadersOn_) {
        s2cb.Projection = XMMatrixTranspose(projection_);
        s2cb.View = XMMatrixTranspose(camera_.getViewMatrix());
    }
    for (const auto& transform : modelTransforms_) {
        if (isEven || !is2ShadersOn_) {
            s1cb.World = XMMatrixTranspose(transform.generateModelMatrix());

            shader1_->use(context_.immediateContext_);
            shader1_->updateConstantBuffer(context_.immediateContext_, s1cb);
            triangle_->draw(context_);
        } else {
            s2cb.World = XMMatrixTranspose(transform.generateModelMatrix());
            
            shader2_->use(context_.immediateContext_);
            shader2_->updateConstantBuffer(context_.immediateContext_, s2cb);
            triangle_->draw(context_);
        }
        isEven = !isEven;
    }

    context_.swapChain_->Present(0, 0);

    if (timeFromStart > benchmarkTime && !isEnd) {
        isEnd = true;
        ex::log(ex::LogLevel::Info, "Frame count after %f seconds", benchmarkTime);
        ex::log(ex::LogLevel::Info, "%d", frameCount_);
    }
}
}