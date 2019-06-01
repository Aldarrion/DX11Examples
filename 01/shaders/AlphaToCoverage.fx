Texture2D diffuseTexture : register(t0);

SamplerState diffuseSampler : register(s0);

struct VS_INPUT {
    float4 Pos : POSITION;
    float2 UV : TEXCOORD0;
};

struct PS_INPUT {
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD1;
};

// =============
// Vertex shader
// =============
PS_INPUT VS(VS_INPUT input) {
    PS_INPUT output = (PS_INPUT) 0;
    
    output.Pos = input.Pos;
    output.UV = input.UV;

    return output;
}

// ============
// Pixel shader
// ============
float4 PS(PS_INPUT input) : SV_Target {
    float4 col = diffuseTexture.Sample(diffuseSampler, input.UV);
    
    /* if (false) {
        // Different thresholds will produce differently sharp cutouts
        const float alphaDiscardThold = 0.2;
        clip(col.a - alphaDiscardThold);
        //col.a = 1.0;
    } else { // Alpha to coverage enabled
        col.a = (col.a - 0.5) / max(fwidth(col.a), 0.0001) + 0.5;
    } */

    //return float4(1, 0, 0, 1);
    //return float4(input.UV.x, input.UV.y, 0, 1);
    return saturate(col);
}
