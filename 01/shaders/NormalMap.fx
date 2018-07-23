#include "PhongLights.fx"

Texture2D DiffuseTexture : register(t0);
Texture2D NormalMap : register(t1);

SamplerState DiffuseSampler : register(s0);
SamplerState PointSampler : register(s1);

cbuffer ConstantBuffer : register(b0) {
    matrix World;
    matrix View;
    matrix Projection;
    matrix NormalMatrix;
    DirLight SunLight;
    float3 ViewPos;
    int UseNormalMap;
}


struct VS_INPUT {
    float4 Pos : POSITION;
    float3 Norm : NORMAL;
    float4 Color : COLOR;
    float2 UV : TEXCOORD0;
    float3 Tangent : TANGENT;
    float3 Bitangent : BINORMAL;
};

struct PS_INPUT {
    float4 Pos : SV_POSITION;
    float3 Norm : TEXCOORD0;
    float3 FragPos : POSITION;
    float2 UV : TEXCOORD1;
    float3 TangentLightPos : TEXCOORD2;
    float3 TangentViewPos : TEXCOORD3;
    float3 TangentFragPos : TEXCOORD4;
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
    output.FragPos = mul(input.Pos, World);
    output.UV = input.UV;

    float3 T = normalize(mul(NormalMatrix, float4(input.Tangent, 1))).xyz;
    float3 N = normalize(mul(NormalMatrix, float4(input.Norm, 1))).xyz;

    T = normalize(T - dot(T, N) * N);
    float3 B = cross(N, T);
    
    float3x3 TBN = transpose(float3x3(T, B, N));

    float3 LightPos = -SunLight.Direction;
    output.TangentLightPos = mul(TBN, LightPos);
    output.TangentViewPos = mul(TBN, ViewPos);
    output.TangentFragPos = mul(TBN, output.FragPos);

    return output;
}


// ============
// Pixel shader
// ============
float4 PS(PS_INPUT input) : SV_Target {
    float3 normal = normalize(input.Norm);
    float3 viewDir = normalize(ViewPos - input.FragPos);
    float4 fragColor = DiffuseTexture.Sample(DiffuseSampler, input.UV);

    float3 fragNormal = normal;
    if (UseNormalMap) {
        fragNormal = NormalMap.Sample(DiffuseSampler, input.UV);
        fragNormal *= 2;
        fragNormal -= float3(1, 1, 1);
        fragNormal.z *= -1;
        fragNormal = normalize(fragNormal);
    }

    float4 finalColor = CalcDirLight(SunLight, fragNormal, fragColor, viewDir);
    finalColor.a = 1.0;

    finalColor = saturate(finalColor);

    //return float4(normal, 1);
    //return fragColor;
    //return float4(fragNormal, 1);
    //return float4(input.FragPos, 1);
    return finalColor;
}
