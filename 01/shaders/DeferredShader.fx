#include "PhongLights.fx"

Texture2D gPosition : register(t0);
Texture2D gNormal : register(t1);
Texture2D gAlbedo : register(t2);

SamplerState txSampler : register(s0);
SamplerState pointSampler : register(s1);

#define NUM_LIGHTS 32

cbuffer ConstantBuffer : register(b0) {
    PointLight lights[NUM_LIGHTS];
    float4 ViewPos;
}

struct VS_INPUT {
    float4 Pos : POSITION;
    float2 UV : TEXCOORD0;
};

struct PS_INPUT {
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

// =============
// Vertex shader
// =============
PS_INPUT VS(VS_INPUT input) {
    PS_INPUT output = (PS_INPUT)0;
    
    output.Pos = input.Pos;
    output.Tex = input.UV;

    return output;
}

// ============
// Pixel shader
// ============
float4 PS(PS_INPUT input) : SV_Target {
    // Retrieve data from gbuffer
    float3 FragPos = gPosition.Sample(pointSampler, input.Tex).rgb;
    float3 Normal = gNormal.Sample(pointSampler, input.Tex).rgb;
    float3 Diffuse = gAlbedo.Sample(pointSampler, input.Tex).rgb;

    // Calculate lighting as usual
    float3 viewDir = normalize(ViewPos - FragPos);

    float4 finalColor = float4(0.0, 0.0, 0.0, 0.0);
    for (int i = 0; i < NUM_LIGHTS; i++) {
        finalColor += CalcPointLight(lights[i], Normal, FragPos, float4(Diffuse, 1.0), viewDir);
    }

    //return float4(Normal, 1);

    return saturate(finalColor);
}
