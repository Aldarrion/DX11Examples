Texture2D txDiffuse : register(t0);

SamplerState diffuseSampler : register(s0);

cbuffer ConstantBuffer : register(b0) {
    matrix Model;
    float4 UVWH;
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
    

    // TODO refactor
    float u, v;
    if (input.UV.x == 0) {
        u = UVWH.x;
    } else {
        u = UVWH.x + UVWH.z;
    }

    if (input.UV.y == 0) {
        v = UVWH.y;
    } else {
        v = UVWH.y + UVWH.w;
    }

    output.Tex = float2(u, v);

    output.Tex = input.UV;

    return output;
}

// ============
// Pixel shader
// ============
float2 lerp(float2 uv) {
    float u = UVWH.x + uv.x * UVWH.z;
    float v = UVWH.y + uv.y * UVWH.w;
    return float2(u, v);
}

float4 PS(PS_INPUT input) : SV_Target {
    float4 outColor = saturate(txDiffuse.Sample(diffuseSampler, lerp(input.Tex)));
    //float4 outColor = saturate(txDiffuse.Sample(diffuseSampler, input.Tex));
    if (outColor.a < 0.01)
        discard;
    return float4(outColor.r, outColor.g, outColor.b, outColor.a);
}
