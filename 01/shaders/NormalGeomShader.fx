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

struct GSPS_INPUT {
    float4 Pos : SV_POSITION;
    float4 NormPos : TEXCOORD0;
};

// =============
// Vertex shader
// =============
GSPS_INPUT VS(VS_INPUT input) {
    GSPS_INPUT output = (GSPS_INPUT)0;
    
    output.Pos = mul(input.Pos, World);
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);

    float MAGNITUDE = 0.4;
    output.NormPos = input.Pos + float4(input.Norm, 0) * MAGNITUDE;
    output.NormPos = mul(output.NormPos, World);
    output.NormPos = mul(output.NormPos, View);
    output.NormPos = mul(output.NormPos, Projection);

    return output;
}

// ===============
// Geometry shader
// ===============
[maxvertexcount(6)]
void GS(triangle GSPS_INPUT input[3], inout LineStream<GSPS_INPUT> lineStream) {
    GSPS_INPUT output = (GSPS_INPUT)0;

    for (int i = 0; i < 3; ++i) {
        output.Pos = input[i].Pos;
        lineStream.Append(output);
        
        output.Pos = input[i].NormPos;
        lineStream.Append(output);
        
        lineStream.RestartStrip();
    }
}

// ============
// Pixel shader
// ============
float4 PS(GSPS_INPUT input) : SV_Target {
    return float4(1.0, 0.0, 0.0, 1.0);
}
