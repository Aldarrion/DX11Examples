cbuffer ConstantBuffer : register(b0) {
    uint4 HistIdx_NumPix; // TODO use HistMask instead of HistIdx
}

StructuredBuffer<uint4> histogram : register(t0);

RWTexture2D<float4> display : register(u0);

static const float4 colors[4] = {
    float4(1, 0, 0, 1),
    float4(0, 1, 0, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 0, 1)
};

[numthreads(2, 256, 1)]
void main(uint3 groupID : SV_GroupID, uint3 threadID : SV_DispatchThreadID) {
    // Just to show a different approach we use fixed texture 256 x 512 size where each level
    // is 2 px wide, therefore each thread group writes a single column and we can use the
    // SV_GroupID semantic to get the group index = histogram level index

    uint count = 0;
    uint value = 0;
    uint maxVal = 0;
    float4 fillCol = float4(0, 0, 0, 1);

    [unroll]
    for (int i = 0; i < 4; ++i) {
        if (HistIdx_NumPix.x & (1 << i)) {
            value += histogram[groupID.x][i];
            maxVal = max(maxVal, histogram[256][i]);
            fillCol.rgb += colors[i].rgb;
            ++count;
        }
    }
    value /= count;

    // All bits are set - use black
    if ((HistIdx_NumPix.x & 7) == 7) {
        fillCol.rgb *= 0;
    }

    uint2 texPos = threadID.xy;

    // By default clear to white
    float4 col = float4(1, 1, 1, 1);
    float texPosToZeroOne = (255 - texPos.y) * (maxVal / 256.0);
    if (value > texPosToZeroOne) {
        col.rgb = fillCol.rgb * (groupID.x / 512.0);
    }

    // Here we are writing the color, the UAV has no type - no SRGB, therefore the SRGB color we
    // work with here will be written to the texture directly
    display[texPos] = col;
}
