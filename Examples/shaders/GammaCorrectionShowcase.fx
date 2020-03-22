#include "Util.h"

cbuffer constantBuffer : register(b0) {
    float4 isGammaCorrectionEnabled;
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
    
    output.Pos = input.Pos;
    output.UV = input.UV;

    return output;
}


// ============
// Pixel shader
// ============
float4 PS(PS_INPUT input) : SV_Target {
    float c = input.UV.x;

    if (!isGammaCorrectionEnabled.x) {
        // Since we are rendering to a SRGB back buffer we can emuate the effect of rendering to a linear back buffer
        // by pretending that the colors are in SRGB and we make them linear.
        // This will produce visually smooth gradient, but assuming we wanted to do physically correct color/light
        // opperations in the shader, this is not correct. E.g., assume this shader tries to show physically correct
        // light gradient, then the gamma correction is necessary and rendering to a linear back buffer does not
        // produce physically correct results.
        c = fromSRGB(c);
    }

    return float4(c, c, c, 1);
}
