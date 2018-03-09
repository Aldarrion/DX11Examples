Texture2D txDiffuse : register(t0);

SamplerState diffuseSampler : register(s0);

cbuffer ConstantBuffer : register(b0) {
    matrix Model;
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

    output.Pos = mul(input.Pos, Model);
    output.Tex = input.UV;

    return output;
}

// ============
// Pixel shader
// ============
float4 PS(PS_INPUT input) : SV_Target{
    float4 outColor = saturate(txDiffuse.Sample(diffuseSampler, input.Tex));
    outColor.a = 1.0;
    return float4(outColor.r, outColor.r, outColor.r, 1.0);
}
