#include "BaseExample.h"
#include "ShaderProgram.h"
#include "Quad.h"
#include "PointWrapSampler.h"
#include "LinearSampler.h"
#include "WinKeyMap.h"
#include "TextSDF.h"

#include <memory>
#include <wrl/client.h>


template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

namespace Compute {

struct HistDataCB {
    DirectX::XMUINT4 Dimensions;
    DirectX::XMFLOAT4 HistPerPix;
};

struct HistDisplCB {
    DirectX::XMUINT4 HistIdx;
};

class HistogramExample : public BaseExample {
public:
    enum HistogramMode {
        HM_RED = 1,
        HM_GREEN = 2,
        HM_BLUE = 4,
        HM_ALL = (HM_RED | HM_GREEN | HM_BLUE)
    };

protected:
    static constexpr int HIST_MODE_COUNT = 4;
    static constexpr HistogramMode HIST_MODES[HIST_MODE_COUNT] = {
        HM_ALL,
        HM_RED,
        HM_GREEN,
        HM_BLUE
    };

    using ConstBuffHistData = ConstBuffer<HistDataCB>;
    using ConstBuffHistDispl = ConstBuffer<HistDisplCB>;

    ComPtr<ID3D11Texture2D> srcTexture_;
    ComPtr<ID3D11ShaderResourceView> srcTextureSRV_;

    ComPtr<ID3D11UnorderedAccessView> histDataUAV_;
    ComPtr<ID3D11ShaderResourceView> histDataSRV_;

    std::unique_ptr<ComputeShader> histDataCS_;
    std::unique_ptr<ComputeShader> histDisplCS_;
    std::unique_ptr<ConstBuffHistData> histDataCB_;

    ComPtr<ID3D11UnorderedAccessView> histDisplUAV_;
    ComPtr<ID3D11ShaderResourceView> histDisplSRV_;

    Shaders::PTexturedQuad texturedQuadShader_;
    std::unique_ptr<ConstBuffHistDispl> histDisplCB_;

    std::unique_ptr<Quad> quad_;
    std::unique_ptr<PointWrapSampler> pointSampler_;
    std::unique_ptr<LinearSampler> linearSampler_;

    int histModeIdx_{ 0 };
    WinKeyMap::WinKeyMap prevHistMode_ = WinKeyMap::Q;
    WinKeyMap::WinKeyMap nextHistMode_ = WinKeyMap::E;

    Text::FontSDF font_;
    std::unique_ptr<Text::TextSDF> infoText_;

    void updateText();
    DirectX::Mouse::Mode getInitialMouseMode() override;
    HRESULT setup() override;
    bool reloadShadersInternal() override;
    void handleInput() override;
    void render() override;
};

}
