Texture2D texSSAO : register(t0);

SamplerState txSampler : register(s0);

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
    uint w, h;
    texSSAO.GetDimensions(w, h);
    float2 texelSize = float2(1.0 / w, 1.0 / h);
    
    float result = 0.0;
    // The noise texture is 4x4 - filter it by 4x4 blur
    for (int x = -2; x < 2; ++x) {
        for (int y = -2; y < 2; ++y) {
            float2 offset = float2(float(x), float(y)) * texelSize;
            result += texSSAO.Sample(txSampler, input.Tex + offset).r;
        }
    }

    float final = result / (4.0 * 4.0);
    return float4(final, final, final, 1.0);
}
