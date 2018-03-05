Texture2D diffuseTexture : register(t0);

SamplerState diffuseSampler : register(s0);

cbuffer ConstantBuffer : register(b0) {
    matrix World;
    matrix View;
    matrix Projection;
    matrix NormalMatrix;
    matrix GrassModels[3];
    matrix GrassPositions[1000];
    int isInstanced;
}

struct VS_INPUT {
    float4 Pos : POSITION;
    uint InstanceId : SV_InstanceID;
};

struct GSPS_INPUT {
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD0;
    uint InstanceId : SV_InstanceID;
};

// =============
// Vertex shader
// =============
GSPS_INPUT VS(VS_INPUT input) {
    GSPS_INPUT output = (GSPS_INPUT)0;
    
    output.Pos = input.Pos;
    output.InstanceId = input.InstanceId;

    return output;
}

struct BillboardVertex {
    float4 posOffset;
    float2 UV;
};

// ===============
// Geometry shader
// ===============
void outputVertex(
    GSPS_INPUT input, 
    inout TriangleStream<GSPS_INPUT> triangleStream, 
    BillboardVertex vertex,
    matrix model
) {
    GSPS_INPUT output = (GSPS_INPUT)0;

    output.Pos = input.Pos + vertex.posOffset;
    output.Pos = mul(output.Pos, model);
    if (isInstanced) {
        output.Pos = mul(output.Pos, GrassPositions[input.InstanceId]);
    } else {
        output.Pos = mul(output.Pos, World);
    }
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);

    output.UV = vertex.UV;

    triangleStream.Append(output);
}

[maxvertexcount(24)]
void GS(point GSPS_INPUT input[1], inout TriangleStream<GSPS_INPUT> triangleStream) {
    for (int i = 0; i < 3; ++i) {
        matrix model = GrassModels[i];

        BillboardVertex topLeft;
        topLeft.posOffset = float4(-1, 1, 0, 1);
        topLeft.UV = float2(0, 0);

        BillboardVertex topRight;
        topRight.posOffset = float4(1, 1, 0, 1);
        topRight.UV = float2(1, 0);

        BillboardVertex botLeft;
        botLeft.posOffset = float4(-1, -1, 0, 1);
        botLeft.UV = float2(0, 1);

        BillboardVertex botRight;
        botRight.posOffset = float4(1, -1, 0, 1);
        botRight.UV = float2(1, 1);

        // Front face
        outputVertex(input[0], triangleStream, topLeft, model);
        outputVertex(input[0], triangleStream, botLeft, model);
        outputVertex(input[0], triangleStream, topRight, model);
        outputVertex(input[0], triangleStream, botRight, model);
        triangleStream.RestartStrip();

        // Back face
        outputVertex(input[0], triangleStream, topRight, model);
        outputVertex(input[0], triangleStream, botRight, model);
        outputVertex(input[0], triangleStream, topLeft, model);
        outputVertex(input[0], triangleStream, botLeft, model);
        triangleStream.RestartStrip();
    }
}

// ============
// Pixel shader
// ============
float4 PS(GSPS_INPUT input) : SV_Target {
    float4 col = saturate(diffuseTexture.Sample(diffuseSampler, input.UV));
    
    // Different thresholds will produce differently sharp cutouts
    const float alphaDiscardThold = 0.2;
    if (col.a < alphaDiscardThold)
        discard; // Disacard so depth is not written
    
    return col;
}
