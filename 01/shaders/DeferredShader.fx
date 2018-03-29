Texture2D gPosition : register(t0);
Texture2D gNormal : register(t1);
Texture2D gAlbedo : register(t2);

SamplerState txSampler : register(s0);
SamplerState pointSampler : register(s1);

cbuffer ConstantBuffer : register(b0) {
    matrix Pad;
    float4 LightPos;
    float4 LightCol;
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

    // Then calculate lighting as usual
    float3 ambient = 0.3 * LightCol.rgb;
    
    // Diffuse
    float3 lightDir = normalize(LightPos.xyz);
    float3 diffuse = max(dot(Normal, lightDir), 0.0) * LightCol.rgb;
    // Specular

    float3 viewDir = normalize(-FragPos);
    float3 reflectDir = reflect(-lightDir, Normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    float3 specular = spec * LightCol.rgb;

    //lighting += diffuse + specular;
    float3 lighting = (ambient + diffuse + specular) * Diffuse;

    //return float4(Normal, 1.0);
    //return LightCol;
    return saturate(float4(lighting, 1.0));
}
