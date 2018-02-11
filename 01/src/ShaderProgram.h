#pragma once
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <iostream>
#include <vector>

template <typename TConstBuffer>
class ShaderProgram {
    ID3D11Buffer* constantBuffer_;
    ID3D11VertexShader* vertexShader_;
    ID3D11GeometryShader* geometryShader_;
    ID3D11PixelShader* pixelShader_;
    ID3D11InputLayout* inputLayout_;

public:
    ShaderProgram(
            ID3D11Device* device, 
            const WCHAR* vertexPath, 
            const char* vertexStart, 
            const WCHAR* pixelPath, 
            const char* pixelStart, 
            const std::vector<D3D11_INPUT_ELEMENT_DESC>& layout,
            const WCHAR* geomPath = nullptr,
            const char* geomStart = nullptr) {
        // Compile vertex shader
        ID3DBlob* VSBlob = nullptr;
        auto hr = CompileShaderFromFile(vertexPath, vertexStart, "vs_5_0", &VSBlob);
        if (FAILED(hr)) {
            MessageBox(nullptr, L"The FX file cannot be compiled (VS). See errors in console.", L"Error", MB_OK);
            return;
        }
        // Create vertex shader
        hr = device->CreateVertexShader(VSBlob->GetBufferPointer(), VSBlob->GetBufferSize(), nullptr, &vertexShader_);
        if (FAILED(hr)) {
            VSBlob->Release();
            MessageBox(nullptr, L"Failed to create vertex shader", L"Error", MB_OK);
            return;
        }
        // Create input layout
        hr = device->CreateInputLayout(layout.data(), static_cast<UINT>(layout.size()), VSBlob->GetBufferPointer(), VSBlob->GetBufferSize(), &inputLayout_);
        VSBlob->Release();
        if (FAILED(hr)) {
            MessageBox(nullptr, L"Failed to create input layout", L"Error", MB_OK);
            return;
        }

        // Compile pixel shader
        ID3DBlob* PSBlob = nullptr;
        hr = CompileShaderFromFile(pixelPath, pixelStart, "ps_5_0", &PSBlob);
        if (FAILED(hr)) {
            MessageBox(nullptr, L"The FX file cannot be compiled (PS). See errors in console.", L"Error", MB_OK);
            return;
        }

        // Create the pixel shader
        hr = device->CreatePixelShader(PSBlob->GetBufferPointer(), PSBlob->GetBufferSize(), nullptr, &pixelShader_);
        PSBlob->Release();
        if (FAILED(hr)) {
            MessageBox(nullptr, L"Failed to create pixel shader", L"Error", MB_OK);
            return;
        }

        // Geometry shader
        if (geomPath != nullptr && geomStart != nullptr) {
            ID3DBlob* GSBlob = nullptr;
            hr = CompileShaderFromFile(geomPath, geomStart, "gs_5_0", &GSBlob);
            if (FAILED(hr)) {
                MessageBox(nullptr, L"The FX file cannot be compiled (GS). See errors in console.", L"Error", MB_OK);
                return;
            }

            D3D11_SO_DECLARATION_ENTRY decl[] = {
                // semantic name, semantic index, start component, component count, output slot
                { 0, "SV_POSITION", 0, 0, 4, 0 },   // output all components of position
                { 1, "TEXCOORD0", 0, 0, 3, 0 },     // output the first 3 of the normal
                { 2, "TEXCOORD1", 0, 0, 2, 0 },     // output the first 2 texture coordinates
            };

            /*hr = device->CreateGeometryShaderWithStreamOutput(
                GSBlob->GetBufferPointer(),
                GSBlob->GetBufferSize(),
                decl,
                sizeof(decl),
                nullptr,
                0,
                0,
                nullptr,
                &geometryShader_
            );*/
            hr = device->CreateGeometryShader(
                GSBlob->GetBufferPointer(),
                GSBlob->GetBufferSize(),
                nullptr,
                &geometryShader_
            );
            GSBlob->Release();
            if (FAILED(hr)) {
                std::cout << hr << std::endl;
                MessageBox(nullptr, L"Failed to create geometry shader", L"Error", MB_OK);
                return;
            }
        } else {
            geometryShader_ = nullptr;
        }

        // Create constantBuffer
        D3D11_BUFFER_DESC bufferDesc;
        ZeroMemory(&bufferDesc, sizeof(bufferDesc));
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.ByteWidth = sizeof(TConstBuffer);
        bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bufferDesc.CPUAccessFlags = 0;
        hr = device->CreateBuffer(&bufferDesc, nullptr, &constantBuffer_);
        if (FAILED(hr)) {
            MessageBox(nullptr, L"Failed to create constant buffer", L"Error", MB_OK);
            return;
        }
    }

    ~ShaderProgram() {
        if (vertexShader_) vertexShader_->Release();
        if (pixelShader_) pixelShader_->Release();
        if (geometryShader_) geometryShader_->Release();
        if (inputLayout_) inputLayout_->Release();
        if (constantBuffer_) constantBuffer_->Release();
    }

    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram operator=(const ShaderProgram&) = delete;

    void use(ID3D11DeviceContext* context) const {
        context->IASetInputLayout(inputLayout_);
        context->VSSetShader(vertexShader_, nullptr, 0);
        context->VSSetConstantBuffers(0, 1, &constantBuffer_);
        context->GSSetShader(geometryShader_, nullptr, 0);
        if (geometryShader_) {
            context->GSSetConstantBuffers(0, 1, &constantBuffer_);
        }
        context->PSSetShader(pixelShader_, nullptr, 0);
        context->PSSetConstantBuffers(0, 1, &constantBuffer_);
    }

    void updateConstantBuffer(ID3D11DeviceContext* context, const TConstBuffer& newBuffer) {
        context->UpdateSubresource(constantBuffer_, 0, nullptr, &newBuffer, 0, 0);
    }

private:
    static HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut) {
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
#endif

        ID3DBlob* pErrorBlob = nullptr;
        hr = D3DCompileFromFile(szFileName, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, szEntryPoint, szShaderModel, dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
        if (FAILED(hr)) {
            if (pErrorBlob) {
                std::cout << "Shader compile errors: " << std::endl;
                std::cout << reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()) << std::endl;
                OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
                pErrorBlob->Release();
            }
            return hr;
        }
        if (pErrorBlob) {
            pErrorBlob->Release();
        }

        return S_OK;
    }
};
