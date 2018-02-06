#include "PhongLights.fx"

Texture2D txDiffuse : register(t0);

SamplerState samLinear : register(s0);

cbuffer ConstantBuffer : register(b0) {
    matrix World;
    matrix View;
    matrix Projection;
    matrix NormalMatrix;
    DirLight DirLights[1];
    float3 ViewPos;
    int DirLightCount;
}


struct VS_INPUT {
    float4 Pos : POSITION;
    float3 Norm : NORMAL;
    float4 Color : COLOR;
    float2 UV : TEXCOORD0;
};

struct PS_INPUT {
    float4 Pos : SV_POSITION;
    float3 Norm : TEXCOORD1;
    float4 Color : COLOR;
    float3 FragPos : POSITION;
    float2 UV : TEXCOORD0;
};


// =============
// Vertex Shader
// =============
PS_INPUT VS(VS_INPUT input) {
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = mul(input.Pos, World);
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);
    output.Norm = mul(float4(input.Norm, 0), NormalMatrix).xyz;
    //output.Norm = input.Norm;
    output.Color = input.Color;
    output.FragPos = mul(input.Pos, World);
    output.UV = input.UV;

    return output;
}


// ============
// Pixel shader
// ============
float4 PS(PS_INPUT input) : SV_Target {
    float3 normal = normalize(input.Norm);
    float3 viewDir = normalize(ViewPos - input.FragPos);
    float4 fragColor = txDiffuse.Sample(samLinear, input.UV);
    
    float4 finalColor = float4(0.0, 0.0, 0.0, 0.0);
    for (int i = 0; i < DirLightCount; ++i) {
        finalColor += CalcDirLight(DirLights[i], normal, fragColor, viewDir);
    }
    /*for (int i = 0; i < PointLightCount; ++i) {
        finalColor += CalcPointLight(PointLights[i], normal, input.FragPos, fragColor, viewDir);
    }
    for (int i = 0; i < SpotLightCount; ++i) {
        finalColor += CalcSpotLight(SpotLights[i], normal, input.FragPos, fragColor, viewDir);
    }*/

    finalColor = saturate(finalColor);

    return finalColor;
    //return float4(normal.x, normal.y, normal.z, 1.0);
}
