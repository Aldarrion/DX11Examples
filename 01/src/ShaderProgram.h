#pragma once
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <iostream>
#include <vector>

template <typename TConstBuffer>
class ShaderProgram {
    ID3D11Buffer* constantBuffer_;
    ID3D11VertexShader* vertexShader_;
    ID3D11PixelShader* pixelShader_;
    ID3D11InputLayout* inputLayout_;

public:
    ShaderProgram(
            ID3D11Device* device, 
            const WCHAR* vertexPath, 
            const char* vertexStart, 
            const WCHAR* pixelPath, 
            const char* pixelStart, 
            const std::vector<D3D11_INPUT_ELEMENT_DESC>& layout) {
        // Compile vertex shader
        ID3DBlob* pVSBlob = nullptr;
        auto hr = CompileShaderFromFile(vertexPath, vertexStart, "vs_4_0", &pVSBlob);
        if (FAILED(hr)) {
            MessageBox(nullptr, L"The FX file cannot be compiled. See errors in console.", L"Error", MB_OK);
            return;
        }
        // Create vertex shader
        hr = device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &vertexShader_);
        if (FAILED(hr)) {
            pVSBlob->Release();
            MessageBox(nullptr, L"Failed to create vertex shader", L"Error", MB_OK);
            return;
        }
        // Create input layout
        hr = device->CreateInputLayout(layout.data(), static_cast<UINT>(layout.size()), pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &inputLayout_);
        pVSBlob->Release();
        if (FAILED(hr)) {
            MessageBox(nullptr, L"Failed to create input layout", L"Error", MB_OK);
            return;
        }
        
        // Compile pixel shader
        ID3DBlob* pPSBlob = nullptr;
        hr = CompileShaderFromFile(pixelPath, pixelStart, "ps_4_0", &pPSBlob);
        if (FAILED(hr)) {
            MessageBox(nullptr, L"The FX file cannot be compiled. See errors in console.", L"Error", MB_OK);
            return;
        }

        // Create the pixel shader
        hr = device->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &pixelShader_);
        pPSBlob->Release();
        if (FAILED(hr)) {
            MessageBox(nullptr, L"Failed to create pixel shader", L"Error", MB_OK);
            return;
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
        if (inputLayout_) inputLayout_->Release();
        if (constantBuffer_) constantBuffer_->Release();
    }

    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram operator=(const ShaderProgram&) = delete;

    void use(ID3D11DeviceContext* context) const {
        context->IASetInputLayout(inputLayout_);
        context->VSSetShader(vertexShader_, nullptr, 0);
        context->VSSetConstantBuffers(0, 1, &constantBuffer_);
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
