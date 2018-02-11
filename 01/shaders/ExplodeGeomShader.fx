Texture2D txDiffuse : register(t0);

SamplerState diffuseSampler : register(s0);

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
    float3 Norm : TEXCOORD0;
    float2 Tex : TEXCOORD1;
};

struct PS_INPUT {
    float4 Pos : SV_POSITION;
};


// =============
// Vertex shader
// =============
GSPS_INPUT VS(VS_INPUT input) {
    GSPS_INPUT output = (GSPS_INPUT)0;
    
    output.Pos = mul(input.Pos, World);
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);
    output.Norm = mul(float4(input.Norm, 0), NormalMatrix).xyz;
    output.Tex = input.UV;

    return output;
}

// ===============
// Geometry shader
// ===============
[maxvertexcount(12)]
void GS(triangle GSPS_INPUT input[3], inout TriangleStream<GSPS_INPUT> TriStream) {
    GSPS_INPUT output;

    float Explode = 1.0;
    //
    // Calculate the face normal
    //
    float3 faceEdgeA = input[1].Pos - input[0].Pos;
    float3 faceEdgeB = input[2].Pos - input[0].Pos;
    float3 faceNormal = normalize(cross(faceEdgeA, faceEdgeB));
    faceNormal = input[0].Norm;
    float3 ExplodeAmt = faceNormal * Explode;

    //
    // Calculate the face center
    //
    float3 centerPos = (input[0].Pos.xyz + input[1].Pos.xyz + input[2].Pos.xyz) / 3.0;
    float2 centerTex = (input[0].Tex + input[1].Tex + input[2].Tex) / 3.0;
    centerPos += faceNormal * Explode;

    //
    // Output the pyramid
    //
    for (int i = 0; i < 3; i++)
    {
        output.Pos = input[i].Pos + float4(ExplodeAmt, 0);
        output.Pos = mul(output.Pos, View);
        output.Pos = mul(output.Pos, Projection);
        output.Norm = input[i].Norm;
        output.Tex = input[i].Tex;
        TriStream.Append(output);

        int iNext = (i + 1) % 3;
        output.Pos = input[iNext].Pos + float4(ExplodeAmt, 0);
        output.Pos = mul(output.Pos, View);
        output.Pos = mul(output.Pos, Projection);
        output.Norm = input[iNext].Norm;
        output.Tex = input[iNext].Tex;
        TriStream.Append(output);

        output.Pos = float4(centerPos, 1) + float4(ExplodeAmt, 0);
        output.Pos = mul(output.Pos, View);
        output.Pos = mul(output.Pos, Projection);
        output.Norm = faceNormal;
        output.Tex = centerTex;
        TriStream.Append(output);

        TriStream.RestartStrip();
    }

    for (int i = 2; i >= 0; i--)
    {
        output.Pos = input[i].Pos + float4(ExplodeAmt, 0);
        output.Pos = mul(output.Pos, View);
        output.Pos = mul(output.Pos, Projection);
        output.Norm = -input[i].Norm;
        output.Tex = input[i].Tex;
        TriStream.Append(output);
    }
    TriStream.RestartStrip();
}

// ============
// Pixel shader
// ============
float4 PS(GSPS_INPUT input) : SV_Target {
    return saturate(txDiffuse.Sample(diffuseSampler, input.Tex));
}
