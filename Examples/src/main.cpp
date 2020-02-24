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
#include "AlphaToCoverageExample.h"
#include "GammaCorrectionExample.h"
#include "SignedDistanceFieldFontExample.h"

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
    std::unique_ptr<Example> example;

    example = std::make_unique<BasicTriangleExample>();
    //example = std::make_unique<Phong::PhongShadingExample>();
    //example = std::make_unique<Texturing::TexturingExample>();
    //example = std::make_unique<Shadows::ShadowsExample>();
    //example = std::make_unique<GeometryShader::GeometryShaderExample>();
    //example = std::make_unique<Billboard::BillboardExample>();
    //example = std::make_unique<Specular::SpecularMapExample>();
    //example = std::make_unique<NormalMap::NormalMappingExample>();
    //example = std::make_unique<FontRendering::FontRenderingExample>();
    //example = std::make_unique<SDF::SignedDistanceFieldFontExample>();
    //example = std::make_unique<ModelLoading::ModelLoadingExample>();
    //example = std::make_unique<Deferred::DeferredRenderingExample>();
    //example = std::make_unique<SSAO::SSAOExample>();

    //example = std::make_unique<ShaderChangePerf::ShaderChangePerfExample>();
    //example = std::make_unique<AlphaToCoverage::AlphaToCoverageExample>();
    //example = std::make_unique<GammaCorrection::GammaCorrectionExample>();
     
    return example->run(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}
