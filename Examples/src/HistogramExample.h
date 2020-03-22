#include "BaseExample.h"
#include "ShaderProgram.h"
#include "Quad.h"
#include "PointWrapSampler.h"

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
    DirectX::XMUINT4 HistIdx_NumPix;
};

class HistogramExample : public BaseExample {
public:

protected:
    using ConstBuffHistData = ConstBuffer<HistDataCB>;
    using ConstBuffHistDispl = ConstBuffer<HistDisplCB>;

    ComPtr<ID3D11Texture2D> srcTexture_;
    ComPtr<ID3D11ShaderResourceView> srcTextureSRV_;

    static constexpr int FRAMES_IN_FLIGHT = 4;
    ComPtr<ID3D11UnorderedAccessView> histDataUAV_[FRAMES_IN_FLIGHT];
    ComPtr<ID3D11ShaderResourceView> histDataSRV_[FRAMES_IN_FLIGHT];

    std::unique_ptr<ComputeShader> histDataCS_;
    std::unique_ptr<ComputeShader> histDisplCS_;
    std::unique_ptr<ConstBuffHistData> histDataCB_;

    ComPtr<ID3D11UnorderedAccessView> histDisplUAV_[FRAMES_IN_FLIGHT];
    ComPtr<ID3D11ShaderResourceView> histDisplSRV_[FRAMES_IN_FLIGHT];

    Shaders::PTexturedQuad histDisplayShader_;
    std::unique_ptr<ConstBuffHistDispl> histDisplCB_;

    std::unique_ptr<Quad> quad_;
    std::unique_ptr<PointWrapSampler> pointSampler_;

    HRESULT setup() override;
    bool reloadShadersInternal() override;
    void handleInput() override;
    void render() override;
};

}
