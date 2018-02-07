#include "PhongLights.fx"

Texture2D txDiffuse : register(t0);
Texture2D ShadowMap : register(t1);

SamplerState samLinear : register(s0);
SamplerState samPoint : register(s1);

cbuffer ConstantBuffer : register(b0) {
    matrix World;
    matrix View;
    matrix Projection;
    matrix NormalMatrix;
    matrix LightView;
    matrix LightProjection;
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
    float4 FragPosLightSpace : POSITION1;
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
    output.Color = input.Color;
    output.FragPos = mul(input.Pos, World);
    output.FragPosLightSpace = mul(float4(output.FragPos, 1.0), LightView);
    output.FragPosLightSpace = mul(output.FragPosLightSpace, LightProjection);
    output.UV = input.UV;

    return output;
}

float ShadowCalc(float4 fragPosLS, DirLight light, float3 normal, float3 fragPos) {
    float3 projCoords = fragPosLS.xyz / fragPosLS.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = ShadowMap.Sample(samLinear, projCoords.xy).r;
    float currentDepth = projCoords.z;
    float3 lightDir = -light.Direction;
    //float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    float bias = 0.0;

    float shadow = 0.0;

    shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    if (projCoords.z < -1.0)
        shadow = 0.0;

    return shadow;
}

// ============
// Pixel shader
// ============
float4 PS(PS_INPUT input) : SV_Target {
    float3 normal = normalize(input.Norm);
    float3 viewDir = normalize(ViewPos - input.FragPos);
    float4 fragColor = txDiffuse.Sample(samLinear, input.UV);
    float4 fragPosLS = input.FragPosLightSpace;
    DirLight light = DirLights[0];
    float3 fragPos = input.FragPos;

    float4 finalColor = float4(0.0, 0.0, 0.0, 0.0);
    //shadow = ShadowCalc(input.FragPosLightSpace, DirLights[i], normal, input.FragPos);
    float3 projCoords = fragPosLS.xyz / fragPosLS.w;
    //projCoords = projCoords * 0.5 + 0.5;
    projCoords.x = projCoords.x * 0.5 + 0.5;
    projCoords.y = projCoords.y * (-0.5) + 0.5;
    //projCoords = saturate(projCoords);
    float closestDepth = ShadowMap.Sample(samPoint, projCoords.xy).r;
    float currentDepth = projCoords.z;
    float3 lightDir = normalize(-light.Direction);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    //bias = 0.0;

    float shadow = 0.0;

    shadow = (currentDepth - bias) > closestDepth ? 1.0 : 0.0;

    if (projCoords.z > 1.0)
        shadow = 0.0;

    finalColor += CalcDirLight(DirLights[0], normal, fragColor, viewDir, shadow);

    finalColor = saturate(finalColor);

    //return float4(shadow, shadow, shadow, shadow);
    //return float4(currentDepth - closestDepth, currentDepth - closestDepth, currentDepth - closestDepth, 1.0);
    //return float4(bias, bias, bias, 1.0);
    //return float4(projCoords, 1.0);
    //return float4(closestDepth, closestDepth, closestDepth, 1.0);
    //return saturate(fragPosLS);
    return finalColor;
    //return float4(normal.x, normal.y, normal.z, 1.0);
    //return ShadowMap.Sample(samLinear, input.UV).rrrr;
}
