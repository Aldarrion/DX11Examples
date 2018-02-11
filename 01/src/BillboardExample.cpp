#include "BillboardExample.h"
#include <directxcolors.h>
#include "Layouts.h"
#include "Transform.h"

namespace Billboard {

using namespace DirectX;

struct PosVertex {
    XMFLOAT4 Position;
};

ID3D11RasterizerState* state = nullptr;

HRESULT BillboardExample::setup() {
    BasicExample::setup();

    textureShader_ = std::make_unique<TextureShader>(
        context_.d3dDevice_,
        L"shaders/UnlitGeom.fx", "VS",
        L"shaders/UnlitGeom.fx", "PS",
        Layouts::TEXTURED_LAYOUT
    );

    plane_ = std::make_unique<Plane>(context_.d3dDevice_);
    seaFloorTexture_ = std::make_unique<Texture>(context_.d3dDevice_, context_.immediateContext_, L"textures/seafloor.dds");
    grassBillboard_ = std::make_unique<Texture>(context_.d3dDevice_, context_.immediateContext_, L"textures/grassBillboard.dds");
    diffuseSampler_ = std::make_unique<AnisotropicSampler>(context_.d3dDevice_);

    std::vector<D3D11_INPUT_ELEMENT_DESC> posLayout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    billboardShader_ = std::make_unique<BillboardShader>(
        context_.d3dDevice_, 
        L"shaders/Billboard.fx", "VS",
        L"shaders/Billboard.fx", "PS",
        posLayout,
        L"shaders/Billboard.fx", "GS"
    );

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
    auto hr = context_.d3dDevice_->CreateBuffer(&bd, &InitData, &vertexBuffer_);
    if (FAILED(hr)) {
        MessageBox(nullptr, L"Failed to create vertex buffer", L"Error", MB_OK);
        return hr;
    }


    // ==================
    // Setup face culling
    // ==================

    D3D11_RASTERIZER_DESC wireframeDesc;
    ZeroMemory(&wireframeDesc, sizeof D3D11_RASTERIZER_DESC);
    wireframeDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
    //wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;

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
        MessageBox(nullptr, L"Failed to create rasterizer state", L"Error", MB_OK);
        return hr;
    }
    context_.immediateContext_->RSSetState(state);

    // =====================
    // Enable alpha blending
    // =====================

    D3D11_BLEND_DESC blendDesc;
    ZeroMemory(&blendDesc, sizeof D3D11_BLEND_DESC);
    blendDesc.RenderTarget[0].BlendEnable = true;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    ID3D11BlendState* blendState;
    hr = context_.d3dDevice_->CreateBlendState(&blendDesc, &blendState);
    if (FAILED(hr)) {
        MessageBox(nullptr, L"Failed to create blend state", L"Error", MB_OK);
        return hr;
    }

    float bl[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    context_.immediateContext_->OMSetBlendState(blendState, bl, 0xffffffff);


    // =========================
    // Setup all grass positions
    // =========================

    const int grassSqrt = static_cast<int>(sqrt(GRASS_COUNT));
    for (int x = -grassSqrt / 2; x < grassSqrt / 2; ++x) {
        for (int z = -grassSqrt / 2; z < grassSqrt / 2; ++z) {
            const float offsetX = static_cast<float>((rand() % 1000) / 1000.0);
            const float offsetZ = static_cast<float>((rand() % 1000) / 1000.0);
            Transform t(XMFLOAT3(x + offsetX, -1.3f, z + offsetZ));
            grassPositions_.push_back(XMMatrixTranspose(t.GenerateModelMatrix()));
        }
    }

    return S_OK;
}

void BillboardExample::render() {
    BasicExample::render();

    std::cout << "Frame time: " << deltaTime_ * 1000 << std::endl;

    const Transform planeTransform(XMFLOAT3(0.0, -4.0f, 0.0f), XMFLOAT3(), XMFLOAT3(20.0f, 2.2f, 20.0f));
    const Transform grassTransform(XMFLOAT3(0.0, -1.3f, 0.0f));
    const Transform grass1Trasform(XMFLOAT3(0, 0.0, 0), XMFLOAT3(0, XMConvertToRadians(90.0), 0));
    const Transform grass2Trasform(XMFLOAT3(0, 0, 0), XMFLOAT3(0, XMConvertToRadians(45.0), 0));

    context_.immediateContext_->ClearRenderTargetView(context_.renderTargetView_, Colors::MidnightBlue);
    context_.immediateContext_->ClearDepthStencilView(context_.depthStencilView_, D3D11_CLEAR_DEPTH, 1.0f, 0);


    BillboardCBuffer cb;
    cb.View = XMMatrixTranspose(camera_.GetViewMatrix());
    cb.Projection = XMMatrixTranspose(projection_);
    
    {
        cb.World = XMMatrixTranspose(planeTransform.GenerateModelMatrix());
        cb.NormalMatrix = computeNormalMatrix(cb.World);
        seaFloorTexture_->use(context_.immediateContext_, 0);
        diffuseSampler_->use(context_.immediateContext_, 0);
        textureShader_->updateConstantBuffer(context_.immediateContext_, cb);
        textureShader_->use(context_.immediateContext_);
        plane_->draw(context_.immediateContext_);
    }

    //for (const auto& t : grassPositions_)
    {
        cb.World = XMMatrixTranspose(grassTransform.GenerateModelMatrix());
        //cb.World = t;
        cb.NormalMatrix = computeNormalMatrix(cb.World);
        cb.GrassModels[0] = XMMatrixIdentity();
        cb.GrassModels[1] = XMMatrixTranspose(grass1Trasform.GenerateModelMatrix());
        cb.GrassModels[2] = XMMatrixTranspose(grass2Trasform.GenerateModelMatrix());
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
        //context_.immediateContext_->Draw(1, 0);
        context_.immediateContext_->DrawInstanced(1, grassPositions_.size(), 0, 0);
    }

    context_.swapChain_->Present(0, 0);
}
}
