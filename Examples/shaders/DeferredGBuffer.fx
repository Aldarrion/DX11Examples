Texture2D texDiffuse : register(t0);

SamplerState txSampler : register(s0);

cbuffer ConstantBuffer : register(b0) {
    matrix World;
    matrix View;
    matrix Projection;
    matrix NormalMatrix;
}


struct VS_INPUT {
    float4 Pos : POSITION;
    float3 Norm : NORMAL;
    float2 UV : TEXCOORD0;
};

struct PS_INPUT {
    float4 Pos : SV_POSITION;
    float3 Norm : TEXCOORD0;
    float3 FragWorldPos : POSITION;
    float2 UV : TEXCOORD1;
};


// =============
// Vertex Shader
// =============
PS_INPUT VS(VS_INPUT input) {
    PS_INPUT output = (PS_INPUT)0;

    output.Pos = mul(input.Pos, World);
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);

    output.Norm = mul(float4(input.Norm, 0), NormalMatrix).xyz;

    output.FragWorldPos = mul(input.Pos, World);

    output.UV = input.UV;

    return output;
}

struct PS_OUTPUT {
    float4 Position : SV_Target0;
    float4 Normal: SV_Target1;
    float4 Color: SV_Target2;
};

// ============
// Pixel shader
// ============
PS_OUTPUT PS(PS_INPUT input) {
    PS_OUTPUT output = (PS_OUTPUT)0;
    output.Position = float4(input.FragWorldPos.xyz, 1);
    // Alpha (or w) has to be 1, otherwise we would read alpha blended normal values
    output.Normal = float4(normalize(input.Norm), 1);
    output.Color = texDiffuse.Sample(txSampler, input.UV);
    //output.Color = float4(0.95, 0.95, 0.95, 1.0);

    return output;
}
