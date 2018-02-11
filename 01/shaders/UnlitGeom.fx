Texture2D txDiffuse : register(t0);

SamplerState diffuseSampler : register(s0);

cbuffer ConstantBuffer : register(b0) {
    matrix World;
    matrix View;
    matrix Projection;
    matrix NormalMatrix;
}

struct VS_INPUT {
    float4 Pos : POSITION;
    float3 Norm : NORMAL;
    float4 Color : COLOR;
    float2 UV : TEXCOORD0;
};

struct PS_INPUT {
    float4 Pos : SV_POSITION;
    float3 Norm : TEXCOORD0;
    float2 Tex : TEXCOORD1;
};

// =============
// Vertex shader
// =============
PS_INPUT VS(VS_INPUT input) {
    PS_INPUT output = (PS_INPUT)0;
    
    output.Pos = mul(input.Pos, World);
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);
    output.Norm = mul(float4(input.Norm, 0), NormalMatrix).xyz;
    output.Tex = input.UV;

    return output;
}

// ============
// Pixel shader
// ============
float4 PS(PS_INPUT input) : SV_Target {
    return saturate(txDiffuse.Sample(diffuseSampler, input.Tex));
}
