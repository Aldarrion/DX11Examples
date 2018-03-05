#include <windows.h>

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

std::unique_ptr<Example> example;

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    example = std::make_unique<BasicTriangleExample>();
    example = std::make_unique<Phong::PhongShadingExample>();
    example = std::make_unique<Texturing::TexturingExample>();
    example = std::make_unique<Shadows::ShadowsExample>();
    example = std::make_unique<GeometryShader::GeometryShaderExample>();
    example = std::make_unique<Billboard::BillboardExample>();
    //example = std::make_unique<Specular::SpecularMapExample>();
    //example = std::make_unique<FontRendering::FontRenderingExample>();

    return example->run(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}
