#include <windows.h>

#include "Example.h"
#include "BasicExample.h"
#include "PhongShadingExample.h"

std::unique_ptr<Example> example;

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    example = std::make_unique<PhongShadingExample>();
    return example->run(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}
