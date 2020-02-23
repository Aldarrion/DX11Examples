Texture2D diffuseTexture : register(t0);

SamplerState diffuseSampler : register(s0);

cbuffer ConstantBuffer : register(b0)
{
    matrix Model;
    float4 UVAdjust;
    float4 Color;
}

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
    
    output.Pos = mul(input.Pos, Model);
    output.UV = (UVAdjust.xy + 0.0015) + input.UV * (UVAdjust.zw - 0.003);

    return output;
}

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

// Anti-aliasing as discussed at https://github.com/Chlumsky/msdfgen/issues/22
float getAlpha(half2 tc, float mdn, float alpha) {
    int2 sz = int2(64, 64);
    float dx = ddx(tc.x) * sz.x;
    float dy = ddy(tc.y) * sz.y;
    float toPixels = /*8.0f;*/ (12 * 1.5f) * rsqrt(dx * dx + dy * dy);
    float sigDist = mdn - alpha;
    float opacity = saturate( sigDist * toPixels + alpha );
    return opacity;
}

// ============
// Pixel shader
// ============
float4 PS(PS_INPUT input) : SV_Target {
    float4 col = diffuseTexture.Sample(diffuseSampler, input.UV);

    float distance = median(col.r, col.g, col.b);
    float alpha = smoothstep(0.2, 0.22, distance);
    
    // Following line makes the font sharper, comment it out and hot-reload shaders with F5
    // to see that even the super-simple variant produces good-looking text.
    alpha = getAlpha(input.UV * 2, distance, 0.2);

    float4 result = float4(Color.r, Color.g, Color.b, Color.a * alpha);

    return result;
}
