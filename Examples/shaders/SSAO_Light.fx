Texture2D gPosition : register(t0);
Texture2D gNormal : register(t1);
Texture2D gAlbedo : register(t2);
Texture2D ssao : register(t3);

SamplerState pointSampler : register(s0);

cbuffer ConstantBuffer : register(b0) {
    float4 LightPos;
    float4 LightCol;
    float4 IsSSAOOn;
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
    PS_INPUT output = (PS_INPUT) 0;
    
    output.Pos = input.Pos;
    output.Tex = input.UV;

    return output;
}

// ============
// Pixel shader
// ============
float4 PS(PS_INPUT input) : SV_Target {
    float3 fragPos = gPosition.Sample(pointSampler, input.Tex).rgb;
    float3 normal = normalize(gNormal.Sample(pointSampler, input.Tex).rgb);
    float3 albedo = gAlbedo.Sample(pointSampler, input.Tex).rgb;
    float ambientOcclusion = ssao.Sample(pointSampler, input.Tex).r;

    // Toggle ssao
    if (!IsSSAOOn.x) {
        ambientOcclusion = 1.0;
    }

    // Claculate lighting in view space
    // SSAO affects only ambient light (ambient occlusion) to simulate global illumination
    float3 ambient = float3(0.3 * albedo * ambientOcclusion);
    float3 viewDir = normalize(-fragPos); // Viewpos in view space is (0, 0, 0)
    
    // Diffuse
    float3 lightDir = normalize(LightPos.xyz - fragPos);
    float3 diffuse = max(dot(normal, lightDir), 0.0) * albedo * LightCol.xyz;
    
    // Blinn-Phong specular
    float3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 8.0);
    float3 specular = LightCol * spec;
    
    float3 lighting = ambient + diffuse + specular;

    return saturate(float4(lighting, 1.0));
}