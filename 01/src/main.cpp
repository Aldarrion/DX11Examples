#include <windows.h>

#include "Example.h"
#include "BasicExample.h"
#include "PhongShadingExample.h"
#include "TexturingExample.h"
#include "ShadowsExample.h"
#include "GeometryShaderExample.h"

std::unique_ptr<Example> example;

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    //example = std::make_unique<PhongShadingExample>();
    //example = std::make_unique<Texturing::TexturingExample>();
    //example = std::make_unique<Shadows::ShadowsExample>();
    example = std::make_unique<GeometryShader::GeometryShaderExample>();
    return example->run(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}
