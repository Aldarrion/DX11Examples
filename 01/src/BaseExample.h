#pragma once
#include "Example.h"
#include "Camera.h"
#include <directxcolors.h>
#include <vector>

class BaseExample : public Example {
public:
    BaseExample();

protected:
    Camera camera_;
    DirectX::XMMATRIX projection_;

    float deltaTime_ = 0.0f;
    float timeFromStart = 0.0f;
    static constexpr float smaPeriod_ = 30;
    float deltaTimeSMA_ = 0.0f;
    size_t frameCount_ = 0;

    HRESULT reloadShaders();
    virtual bool reloadShadersInternal() { return true; }
    virtual void handleInput();
    HRESULT setup() override;
    void render() override;

    static DirectX::XMMATRIX computeNormalMatrix(const DirectX::XMMATRIX& model);
    static DirectX::XMMATRIX computeNormalMatrix(const std::vector<DirectX::XMMATRIX>& matrices);

    void clearViews() const;
};
