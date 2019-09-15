Texture2D diffuseTexture : register(t0);

SamplerState diffuseSampler : register(s0);

cbuffer ConstantBuffer : register(b0)
{
    matrix Model;
    float4 UVAdjust;
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
    output.UV = UVAdjust.xy + input.UV * UVAdjust.zw;

    return output;
}

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

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
    float distance = col.r;

    float4 result = float4(0.0, 0.0, 0.0, 0.0);
    /*if (distance > 0.5)
        result = float4(1.0, 1.0, 1.0, 1.0);*/


    float alpha = smoothstep(0.2, 0.25, distance);

    //float distance = median(col.r, col.g, col.b);
    //sigDist *= dot(msdfUnit, 0.5 / fwidth(pos));
    //float alpha = clamp(sigDist + 0.5, 0.0, 1.0);
    float smoothWidth = fwidth(distance);
    //distance *= dot(msdfUnit, 0.5 / fwidth(input.UV));
    //float alpha = smoothstep(0.5 - smoothWidth, 0.5 + smoothWidth, distance);

    alpha = getAlpha(input.UV * 2, distance, 0.2);

    result = float4(1.0, 1.0, 1.0, alpha);

    return result;
}
