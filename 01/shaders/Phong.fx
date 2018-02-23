#include "PhongLights.fx"

cbuffer ConstantBuffer : register(b0) {
    matrix World;
    matrix View;
    matrix Projection;
    matrix NormalMatrix;
    PointLight PointLights[2];
    DirLight DirLights[2];
    SpotLight SpotLights[2];
    float3 ViewPos;
    int PointLightCount;
    int DirLightCount;
    int SpotLightCount;
}


struct VS_INPUT {
    float4 Pos : POSITION;
    float3 Norm : NORMAL;
    float4 Color : COLOR;
};

struct PS_INPUT {
    float4 Pos : SV_POSITION;
    float3 Norm : TEXCOORD0;
    float4 Color : COLOR;
    float3 FragPos : POSITION;
};


// =============
// Vertex Shader
// =============
PS_INPUT VS(VS_INPUT input) {
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = mul(input.Pos, World);
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);
    output.Norm = mul(float4(input.Norm, 1), NormalMatrix).xyz;
    output.Color = input.Color;
    output.FragPos = mul(input.Pos, World);

    return output;
}


// ============
// Pixel shader
// ============
float4 PS(PS_INPUT input) : SV_Target {
    float3 normal = normalize(input.Norm);
    float3 viewDir = normalize(ViewPos - input.FragPos);
    
    // Iterate all through all lights and add their contribution
    float4 finalColor = float4(0.0, 0.0, 0.0, 0.0);
    for (int i = 0; i < DirLightCount; ++i) {
        finalColor += CalcDirLight(DirLights[i], normal, input.Color, viewDir);
    }
    for (int i = 0; i < PointLightCount; ++i) {
        finalColor += CalcPointLight(PointLights[i], normal, input.FragPos, input.Color, viewDir);
    }
    for (int i = 0; i < SpotLightCount; ++i) {
        finalColor += CalcSpotLight(SpotLights[i], normal, input.FragPos, input.Color, viewDir);
    }

    // clamp to (0,1)
    finalColor = saturate(finalColor);

    return finalColor;
}
