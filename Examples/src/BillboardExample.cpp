#include "BillboardExample.h"
#include <directxcolors.h>
#include "Layouts.h"
#include "Transform.h"
#include "WinKeyMap.h"

// Uncomment to see non instanced version
// Recommended to run in Release mode
#define BILLBOARD_EXAMPLE_NON_INSTANCED

namespace Billboard {

using namespace DirectX;

struct PosVertex {
    XMFLOAT4 Position;
};

ID3D11RasterizerState* state = nullptr;

HRESULT BillboardExample::setup() {
    auto hr = BaseExample::setup();
    if (FAILED(hr))
        return hr;

    Text::makeDefaultSDFFont(context_, font_);
    frameTimeText_ = std::make_unique<Text::TextSDF>("Frame time: 0", &font_);

    plane_ = std::make_unique<Plane>(context_.d3dDevice_);
    seaFloorTexture_ = std::make_unique<Texture>(context_.d3dDevice_, context_.immediateContext_, L"textures/seafloor.dds", true);
    grassBillboard_ = std::make_unique<Texture>(context_.d3dDevice_, context_.immediateContext_, L"textures/grassBillboard.dds", true);
    diffuseSampler_ = std::make_unique<AnisotropicSampler>(context_.d3dDevice_);

    hr = reloadShaders();
    if (FAILED(hr))
        return hr;

    // ==============================
    // Setup point for grass position
    // ==============================

    std::vector<PosVertex> vertices = {
        { XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) }
    };

    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(PosVertex);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices.data();
    hr = context_.d3dDevice_->CreateBuffer(&bd, &InitData, &vertexBuffer_);
    if (FAILED(hr)) {
        MessageBoxA(nullptr, "Failed to create vertex buffer", "Error", MB_OK);
        return hr;
    }


    // ==================
    // Setup face culling
    // ==================

    D3D11_RASTERIZER_DESC CurrentRasterizerState;
    CurrentRasterizerState.FillMode = D3D11_FILL_SOLID;
    CurrentRasterizerState.CullMode = D3D11_CULL_FRONT;
    CurrentRasterizerState.FrontCounterClockwise = true;
    CurrentRasterizerState.DepthBias = false;
    CurrentRasterizerState.DepthBiasClamp = 0;
    CurrentRasterizerState.SlopeScaledDepthBias = 0;
    CurrentRasterizerState.DepthClipEnable = true;
    CurrentRasterizerState.ScissorEnable = false;
    CurrentRasterizerState.MultisampleEnable = true;
    CurrentRasterizerState.AntialiasedLineEnable = false;
    
    hr = context_.d3dDevice_->CreateRasterizerState(&CurrentRasterizerState, &state);
    if (FAILED(hr)) {
        MessageBoxA(nullptr, "Failed to create rasterizer state", "Error", MB_OK);
        return hr;
    }
    context_.immediateContext_->RSSetState(state);

    // =====================
    // Enable alpha blending
    // =====================

    D3D11_BLEND_DESC blendDesc;
    ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
    blendDesc.AlphaToCoverageEnable = true;
    blendDesc.RenderTarget[0].BlendEnable = false;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    hr = context_.d3dDevice_->CreateBlendState(&blendDesc, &blendState_);
    if (FAILED(hr)) {
        MessageBoxA(nullptr, "Failed to create blend state", "Error", MB_OK);
        return hr;
    }




    // =========================
    // Setup all grass positions
    // =========================

    const int grassSqrt = static_cast<int>(sqrt(GRASS_COUNT));
    for (int x = -grassSqrt / 2; x < grassSqrt / 2; ++x) {
        for (int z = -grassSqrt / 2; z < grassSqrt / 2; ++z) {
            const float offsetX = static_cast<float>((rand() % 1000) / 1000.0);
            const float offsetZ = static_cast<float>((rand() % 1000) / 1000.0);
            Transform t(XMFLOAT3(x + offsetX, -1.3f, z + offsetZ));
            grassPositions_.push_back(XMMatrixTranspose(t.generateModelMatrix()));
        }
    }

    return S_OK;
}

bool BillboardExample::reloadShadersInternal() {
    std::vector<D3D11_INPUT_ELEMENT_DESC> posLayout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    return
        Shaders::makeShader<TextureShader>(
        textureShader_,
        context_.d3dDevice_,
        "shaders/UnlitGeom.fx", "VS",
        "shaders/UnlitGeom.fx", "PS",
        Layouts::TEXTURED_LAYOUT)

    && Shaders::makeShader<BillboardShader>(
        billboardShader_,
        context_.d3dDevice_,
        "shaders/Billboard.fx", "VS",
        "shaders/Billboard.fx", "PS",
        posLayout,
        "shaders/Billboard.fx", "GS"
    );
}

void BillboardExample::handleInput() {
    BaseExample::handleInput();
    if (GetAsyncKeyState(WinKeyMap::E) & 1) {
        isInstanced_ = !isInstanced_;
    }
}

void BillboardExample::render() {
    BaseExample::render();

    frameTimeText_->setText("\n Frame time (ms): " + std::to_string(deltaTimeSMA_ * 1000) +
        "\n E: toggle instanced rendering. Is instanced: " + std::to_string(isInstanced_)
    );

    const Transform planeTransform(XMFLOAT3(0.0, -4.0f, 0.0f), XMFLOAT3(), XMFLOAT3(20.0f, 2.2f, 20.0f));
    const Transform grassTransform(XMFLOAT3(0.0, -1.3f, 0.0f));
    const Transform grass1Trasform(XMFLOAT3(0, 0.0, 0), XMFLOAT3(0, XMConvertToRadians(90.0), 0));
    const Transform grass2Trasform(XMFLOAT3(0, 0, 0), XMFLOAT3(0, XMConvertToRadians(45.0), 0));

    context_.immediateContext_->ClearRenderTargetView(context_.renderTargetView_, Util::srgbToLinear(DirectX::Colors::MidnightBlue));
    context_.immediateContext_->ClearDepthStencilView(context_.depthStencilView_, D3D11_CLEAR_DEPTH, 1.0f, 0);

    float bl[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    context_.immediateContext_->OMSetBlendState(blendState_, bl, 0xffffffff);

    BillboardCBuffer cb;
    cb.View = XMMatrixTranspose(camera_.getViewMatrix());
    cb.Projection = XMMatrixTranspose(projection_);
    
    {
        cb.World = XMMatrixTranspose(planeTransform.generateModelMatrix());
        cb.NormalMatrix = computeNormalMatrix(cb.World);
        seaFloorTexture_->use(context_.immediateContext_, 0);
        diffuseSampler_->use(context_.immediateContext_, 0);
        textureShader_->updateConstantBuffer(context_.immediateContext_, cb);
        textureShader_->use(context_.immediateContext_);
        plane_->draw(context_.immediateContext_);
    }

    // Instanced rendering breaks out of this loop
    for (const auto& t : grassPositions_)
    {
        if (isInstanced_) {
            cb.World = XMMatrixTranspose(grassTransform.generateModelMatrix());
            cb.IsInstanced = 1;
        } else {
            cb.World = t;
            cb.IsInstanced = 0;
        }
        cb.NormalMatrix = computeNormalMatrix(cb.World);
        cb.GrassModels[0] = XMMatrixIdentity();
        cb.GrassModels[1] = XMMatrixTranspose(grass1Trasform.generateModelMatrix());
        cb.GrassModels[2] = XMMatrixTranspose(grass2Trasform.generateModelMatrix());
        for (int i = 0; i < grassPositions_.size(); ++i) {
            cb.GrassMatrices[i] = grassPositions_[i];
        }
        grassBillboard_->use(context_.immediateContext_, 0);
        diffuseSampler_->use(context_.immediateContext_, 0);
        billboardShader_->updateConstantBuffer(context_.immediateContext_, cb);
        billboardShader_->use(context_.immediateContext_);

        UINT stride = sizeof(PosVertex);
        UINT offset = 0;
        context_.immediateContext_->IASetVertexBuffers(0, 1, &vertexBuffer_, &stride, &offset);
        context_.immediateContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
        if (isInstanced_) {
            context_.immediateContext_->DrawInstanced(1, static_cast<UINT>(grassPositions_.size()), 0, 0);
            break; // Instanced rendering does not use the for loop
        } else {
            context_.immediateContext_->Draw(1, 0);
        }
    }

    frameTimeText_->draw(context_);

    context_.swapChain_->Present(0, 0);
}
}
