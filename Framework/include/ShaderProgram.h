#pragma once
#include "ResourceHolder.h"
#include "Layouts.h"
#include "ConstantBuffers.h"
#include "ContextWrapper.h"
#include "Logging.h"

#include <array>
#include <vector>
#include <d3dcompiler.h>
#include <memory>

namespace Shaders {

extern bool isLastCompileOK;

inline HRESULT CompileShaderFromFile(const char* fileName, const char* entryPoint, const char* shaderModel, ID3DBlob** ppBlobOut) {
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
    #ifdef _DEBUG
        // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
        // Setting this flag improves the shader debugging experience, but still allows 
        // the shaders to be optimized and to run exactly the way they will run in 
        // the release configuration of this program.
        dwShaderFlags |= D3DCOMPILE_DEBUG;

        // Disable optimizations to further improve shader debugging
        dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
    #else
        // We have pretty small shaders - O3 should not take too long
        dwShaderFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
    #endif

    ex::log(ex::LogLevel::Info, "Compiling: %s | %s", fileName, entryPoint);
        
    auto fileNameLen = strlen(fileName);
    std::wstring wFileName(fileNameLen, L'x');
    auto convertedLen = mbstowcs(&wFileName[0], fileName, fileNameLen);
    assert(convertedLen >= 0 && "Error while converting string to wstring");

    ID3DBlob* pErrorBlob = nullptr;
    hr = D3DCompileFromFile(wFileName.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, shaderModel, dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
        
    if (FAILED(hr)) {
        if (pErrorBlob) {
            ex::log(ex::LogLevel::Error, "Shader compile: ");
            ex::log(ex::LogLevel::Error, "%s", reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
            pErrorBlob->Release();
        }
        Shaders::isLastCompileOK = false;
        return hr;
    }
    if (pErrorBlob) {
        pErrorBlob->Release();
    }

    Shaders::isLastCompileOK = true;
    ex::log(ex::LogLevel::Info, "Success!");
    return S_OK;
}

}

template<typename ... TCBuffers>
class ShaderProgram : public ResourceHolder {
    typedef std::tuple<TCBuffers ...> types;
    typedef std::array<ID3D11Buffer*, sizeof...(TCBuffers)> buffer_array_t;

    buffer_array_t cbuffers_{};
    ID3D11VertexShader* vertexShader_{};
    ID3D11GeometryShader* geometryShader_{};
    ID3D11PixelShader* pixelShader_{};
    ID3D11InputLayout* inputLayout_{};

    template<int ArrSize, int N>
    struct InitCb {
        static void initCb(ID3D11Device* device, buffer_array_t& arr) {
            // Create constantBuffer
            D3D11_BUFFER_DESC bufferDesc{};
            bufferDesc.Usage = D3D11_USAGE_DEFAULT;
            bufferDesc.ByteWidth = sizeof(typename std::tuple_element<N, types>::type);
            bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bufferDesc.CPUAccessFlags = 0;
            auto hr = device->CreateBuffer(&bufferDesc, nullptr, &arr[N]);
            if (FAILED(hr)) {
                assert(!"Failed to create constant buffer");
                return;
            }

            InitCb<ArrSize, N + 1>::initCb(device, arr);
        }
    };

    template<int ArrSize>
    struct InitCb<ArrSize, ArrSize> {
        static void initCb(ID3D11Device*, buffer_array_t&) {
        }
    };


public:
    ShaderProgram(
            ID3D11Device* device,
            const char* vertexPath,
            const char* vertexStart,
            const char* pixelPath,
            const char* pixelStart,
            const std::vector<D3D11_INPUT_ELEMENT_DESC>& layout,
            const char* geomPath = nullptr,
            const char* geomStart = nullptr
    ) {
        // Compile vertex shader
        ID3DBlob* VSBlob = nullptr;

        auto hr = Shaders::CompileShaderFromFile(vertexPath, vertexStart, "vs_5_0", &VSBlob);
        if (FAILED(hr)) {
            //MessageBox(nullptr, L"The FX file cannot be compiled (VS). See errors in console.", L"Error", MB_OK);
            return;
        }
        // Create vertex shader
        hr = device->CreateVertexShader(VSBlob->GetBufferPointer(), VSBlob->GetBufferSize(), nullptr, &vertexShader_);
        if (FAILED(hr)) {
            VSBlob->Release();
            //MessageBox(nullptr, L"Failed to create vertex shader", L"Error", MB_OK);
            return;
        }
        // Create input layout
        hr = device->CreateInputLayout(layout.data(), static_cast<UINT>(layout.size()), VSBlob->GetBufferPointer(), VSBlob->GetBufferSize(), &inputLayout_);
        VSBlob->Release();
        if (FAILED(hr)) {
            //MessageBox(nullptr, L"Failed to create input layout", L"Error", MB_OK);
            return;
        }

        // Compile pixel shader
        ID3DBlob* PSBlob = nullptr;
        hr = Shaders::CompileShaderFromFile(pixelPath, pixelStart, "ps_5_0", &PSBlob);
        if (FAILED(hr)) {
            //MessageBox(nullptr, L"The FX file cannot be compiled (PS). See errors in console.", L"Error", MB_OK);
            return;
        }

        // Create the pixel shader
        hr = device->CreatePixelShader(PSBlob->GetBufferPointer(), PSBlob->GetBufferSize(), nullptr, &pixelShader_);
        PSBlob->Release();
        if (FAILED(hr)) {
            //MessageBox(nullptr, L"Failed to create pixel shader", L"Error", MB_OK);
            return;
        }

        // Geometry shader
        if (geomPath != nullptr && geomStart != nullptr) {
            ID3DBlob* GSBlob = nullptr;
            hr = Shaders::CompileShaderFromFile(geomPath, geomStart, "gs_5_0", &GSBlob);
            if (FAILED(hr)) {
                //MessageBox(nullptr, L"The FX file cannot be compiled (GS). See errors in console.", L"Error", MB_OK);
                return;
            }

            D3D11_SO_DECLARATION_ENTRY decl[] = {
                // semantic name, semantic index, start component, component count, output slot
                { 0, "SV_POSITION", 0, 0, 4, 0 },   // output all components of position
                { 1, "TEXCOORD0", 0, 0, 3, 0 },     // output the first 3 of the normal
                { 2, "TEXCOORD1", 0, 0, 2, 0 },     // output the first 2 texture coordinates
            };

            hr = device->CreateGeometryShader(
                GSBlob->GetBufferPointer(),
                GSBlob->GetBufferSize(),
                nullptr,
                &geometryShader_
            );
            GSBlob->Release();
            if (FAILED(hr)) {
                ex::log(ex::LogLevel::Error, "Failed to create geometry shader %d", hr);
                assert(!"Failed to create geometry shader");
                return;
            }
        } else {
            geometryShader_ = nullptr;
        }

        InitCb<sizeof...(TCBuffers), 0>::initCb(device, cbuffers_);
    }

    ~ShaderProgram() {
        if (vertexShader_) 
            vertexShader_->Release();
        if (pixelShader_) 
            pixelShader_->Release();
        if (geometryShader_) 
            geometryShader_->Release();
        if (inputLayout_) 
            inputLayout_->Release();
        if (!cbuffers_.empty()) {
            for (auto cbuffer : cbuffers_) {
                if (cbuffer)
                    cbuffer->Release();
            }
        }
    }

    void use(ID3D11DeviceContext* context) const {
        context->IASetInputLayout(inputLayout_);
        context->VSSetShader(vertexShader_, nullptr, 0);
        if (!cbuffers_.empty()) {
            for (auto cbuffer : cbuffers_) {
                context->VSSetConstantBuffers(0, 1, &cbuffer);
            }
        }
        context->GSSetShader(geometryShader_, nullptr, 0);
        if (geometryShader_ && !cbuffers_.empty()) {
            for (auto cbuffer : cbuffers_) {
                context->GSSetConstantBuffers(0, 1, &cbuffer);
            }
        }
        context->PSSetShader(pixelShader_, nullptr, 0);
        if (!cbuffers_.empty()) {
            for (auto cbuffer : cbuffers_) {
                context->PSSetConstantBuffers(0, 1, &cbuffer);
            }
        }
    }

private:
    template<int N, typename T>
    struct BuffOfType : std::is_same<T, typename std::tuple_element<N, types>::type>
    { };

    template<int N, typename TBuff, bool IsMatch>
    struct MatchingBuff {
        static ID3D11Buffer* get(buffer_array_t& arr) {
            return MatchingBuff<N + 1, TBuff, BuffOfType<N + 1, TBuff>::value>::get(arr);
        }
    };

    template<int N, typename TBuff>
    struct MatchingBuff<N, TBuff, true> {
        static ID3D11Buffer* get(buffer_array_t& arr) {
            return arr[N];
        }
    };

    template<typename TBuff>
    struct GetBuffer {
        static ID3D11Buffer* get(buffer_array_t& arr) {
            return MatchingBuff<0, TBuff, BuffOfType<0, TBuff>::value>::get(arr);
        }
    };

public:
    template<typename TConstBuffer>
    void updateConstantBuffer(ID3D11DeviceContext* context, const TConstBuffer& newBuffer) {
        auto buffer = GetBuffer<TConstBuffer>::get(cbuffers_);
        context->UpdateSubresource(buffer, 0, nullptr, &newBuffer, 0, 0);
    }

};

enum class Stage {
    VS,
    GS,
    PS,
    CS,
};

template<typename TBuff>
class ConstBuffer {
public:
    ConstBuffer(ID3D11Device* device) {
        D3D11_BUFFER_DESC bufferDesc{};
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.ByteWidth = sizeof(TBuff);
        bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bufferDesc.CPUAccessFlags = 0;
        auto hr = device->CreateBuffer(&bufferDesc, nullptr, &cbuffer_);
        if (FAILED(hr)) {
            assert(!"Failed to create constant buffer");
            return;
        }

    }

    ~ConstBuffer() {
        if (cbuffer_)
            cbuffer_->Release();
    }

    void update(ID3D11DeviceContext* context, const TBuff& data) {
        context->UpdateSubresource(cbuffer_, 0, nullptr, &data, 0, 0);
    }

    template<Stage stage>
    void use(ID3D11DeviceContext* context, int slot) const {
        if (stage == Stage::VS) {
            context->VSSetConstantBuffers(slot, 1, &cbuffer_);
        } else if (stage == Stage::GS) {
            context->GSSetConstantBuffers(slot, 1, &cbuffer_);
        } else if (stage == Stage::PS) {
            context->PSSetConstantBuffers(slot, 1, &cbuffer_);
        } else if (stage == Stage::CS) {
            context->CSSetConstantBuffers(slot, 1, &cbuffer_);
        }
    }

private:
    ID3D11Buffer* cbuffer_{};
};

class ComputeShader {
public:
    ComputeShader(ID3D11Device* device, const char* path, const char* start) {
        ID3DBlob* result;
        if (FAILED(Shaders::CompileShaderFromFile(path, start, "cs_5_0", &result))) {
            return;
        }

        device->CreateComputeShader(result->GetBufferPointer(), result->GetBufferSize(), nullptr, &shader_);
        result->Release();
    }

    ~ComputeShader() {
        if (shader_)
            shader_->Release();
    }

    void use(ID3D11DeviceContext* context) {
        if (shader_) {
            context->CSSetShader(shader_, nullptr, 0);
        }
    }

private:
    ID3D11ComputeShader* shader_{};
};

namespace Shaders {
    using SolidShader = ShaderProgram<ConstantBuffers::SolidConstBuffer>;
    using PSolidShader = std::unique_ptr<SolidShader>;
    inline PSolidShader createSolidShader(const ContextWrapper& context) {
        return std::make_unique<SolidShader>(context.d3dDevice_, "shaders/Solid.fx", "VS", "shaders/Solid.fx", "PSSolid", Layouts::POS_NORM_COL_LAYOUT);
    }

    template<typename ShaderT>
    inline bool makeShader(
        std::unique_ptr<ShaderT>& oldShader,
        ID3D11Device* device,
        const char* vertexPath,
        const char* vertexStart,
        const char* pixelPath,
        const char* pixelStart,
        const std::vector<D3D11_INPUT_ELEMENT_DESC>& layout,
        const char* geomPath = nullptr,
        const char* geomStart = nullptr
    ) {
        auto shader = std::make_unique<ShaderT>(device, vertexPath, vertexStart, pixelPath, pixelStart, layout, geomPath, geomStart);
        if (Shaders::isLastCompileOK) {
            oldShader = std::move(shader);
            return true;
        }
        
        return false;
    }

    inline bool makeSolidShader(PSolidShader& oldShader, const ContextWrapper& context) {
        return makeShader<SolidShader>(oldShader, context.d3dDevice_, "shaders/Solid.fx", "VS", "shaders/Solid.fx", "PSSolid", Layouts::POS_NORM_COL_LAYOUT);
    }

    struct TexturedQuadCB {
        DirectX::XMMATRIX World;
    };
    using TexturedQuad = ShaderProgram<TexturedQuadCB>;
    using PTexturedQuad = std::unique_ptr<TexturedQuad>;
}
