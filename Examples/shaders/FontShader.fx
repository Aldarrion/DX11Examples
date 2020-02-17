Texture2D txDiffuse : register(t0);

SamplerState diffuseSampler : register(s0);

cbuffer ConstantBuffer : register(b0) {
    matrix Model;
    float4 UVWH;
    float4 TextColor;
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
    
    // Compute UV for given glyph in the texture atlas
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

    return output;
}

// ============
// Pixel shader
// ============
float4 PS(PS_INPUT input) : SV_Target {
    float4 outColor = saturate(txDiffuse.Sample(diffuseSampler, input.Tex));
    clip(outColor.a - 0.01);

    outColor *= TextColor;
    return outColor;
}
