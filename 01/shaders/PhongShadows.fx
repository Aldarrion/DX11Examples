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
    DirLight SunLight;
    float3 ViewPos;
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

float ShadowCalc(float4 fragPosLS, DirLight light, float3 normal, float3 fragPos, bool softShadows) {
    float3 projCoords = fragPosLS.xyz / fragPosLS.w;
    projCoords.x = projCoords.x * 0.5 + 0.5;
    projCoords.y = projCoords.y * (-0.5) + 0.5;
    float currentDepth = projCoords.z;
    float3 lightDir = normalize(-light.Direction);
    // Bias is fine tuned to this particular case
    float bias = max(0.003 * (1.0 - dot(normal, lightDir)), 0.0005);
    //bias = 0.0; // Uncomment this to see why we need bias

    if (projCoords.z > 1.0)
        return 0.0;

    float shadow = 0.0;
    if (softShadows) {
        uint w, h;
        ShadowMap.GetDimensions(w, h);
        float2 texelSize = float2(1.0 / w, 1.0 / h);
        
        // Average 9 shadow map texels around our current pixel
        for (int x = -1; x <= 1; ++x) {
            for (int y = -1; y <= 1; ++y) {
                float closestDepth = ShadowMap.Sample(samPoint, projCoords.xy + float2(x * texelSize.x, y * texelSize.y)).r;
                shadow += (currentDepth - bias) > closestDepth ? 1.0 : 0.0;
            }
        }
        shadow /= 9.0;
    } else {
        float closestDepth = ShadowMap.Sample(samPoint, projCoords.xy).r;
        shadow = (currentDepth - bias) > closestDepth ? 1.0 : 0.0;
    }

    return shadow;
}

// ============
// Pixel shader
// ============
float4 PS(PS_INPUT input) : SV_Target {
    float3 normal = normalize(input.Norm);
    float3 viewDir = normalize(ViewPos - input.FragPos);
    float4 fragColor = txDiffuse.Sample(samLinear, input.UV);

    float shadow = ShadowCalc(input.FragPosLightSpace, SunLight, normal, input.FragPos, true);

    float4 finalColor = CalcDirLight(SunLight, normal, fragColor, viewDir, shadow);

    return saturate(finalColor);
}
