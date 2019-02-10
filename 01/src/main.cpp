#include "Example.h"
#include "BaseExample.h"
#include "PhongShadingExample.h"
#include "TexturingExample.h"
#include "ShadowsExample.h"
#include "GeometryShaderExample.h"
#include "BillboardExample.h"
#include "BasicTriangleExample.h"
#include "SpecularMapExample.h"
#include "FontRenderingExample.h"
#include "ModelLoadingExample.h"
#include "DeferredRenderingExample.h"
#include "SSAOExample.h"
#include "NormalMappingExample.h"
#include "ShaderChangePerfExample.h"

std::unique_ptr<Example> example;

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    //example = std::make_unique<BasicTriangleExample>();
    //example = std::make_unique<Phong::PhongShadingExample>();
    //example = std::make_unique<Texturing::TexturingExample>();
    //example = std::make_unique<Shadows::ShadowsExample>();
    //example = std::make_unique<GeometryShader::GeometryShaderExample>();
    //example = std::make_unique<Billboard::BillboardExample>();
    //example = std::make_unique<Specular::SpecularMapExample>();
    //example = std::make_unique<NormalMap::NormalMappingExample>();
    //example = std::make_unique<FontRendering::FontRenderingExample>();
    //example = std::make_unique<ModelLoading::ModelLoadingExample>();
    //example = std::make_unique<Deferred::DeferredRenderingExample>();
    example = std::make_unique<SSAO::SSAOExample>();

    //example = std::make_unique<ShaderChangePerf::ShaderChangePerfExample>();

    return example->run(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}
