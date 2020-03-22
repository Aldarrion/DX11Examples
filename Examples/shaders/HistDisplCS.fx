cbuffer ConstantBuffer : register(b0) {
    uint4 HistIdx_NumPix; // TODO use HistMask instead of HistIdx
}

StructuredBuffer<uint4> histogram : register(t0);

RWTexture2D<float4> display : register(u0);

float4 colors[4] = {
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
    uint2 texPos = threadID.xy;

    // By default clear to white
    float4 col = float4(1, 1, 1, 1);
    if (histogram[groupID.x][HistIdx_NumPix.x] > (255 - texPos.y) * (HistIdx_NumPix.y / 256.0)) {
        col.rbg = colors[HistIdx_NumPix.x] * (groupID.x / 512.0);
    }

    // Here we are writing the color, the UAV has no type - no SRGB, therefore the SRGB color we
    // work with here will be written to the texture directly
    display[texPos] = col;
}
