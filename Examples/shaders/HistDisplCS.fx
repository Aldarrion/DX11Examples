cbuffer ConstantBuffer : register(b0) {
    uint4 HistMask; // x = mask of which colors should be shown
}

// Input buffer with the histogram calculated in previous compute shader
StructuredBuffer<uint4> histogram : register(t0);

// Texture we craft here showing the actual histogram displayed later on screen
RWTexture2D<float4> display : register(u0);

static const float4 colors[4] = {
    float4(1, 0, 0, 1),
    float4(0, 1, 0, 1),
    float4(0, 0, 1, 1),
    float4(0, 0, 0, 1)
};

// This value is shared between all threads in each group
groupshared uint4 maxValues;

// Just to show a different approach we use fixed texture 256 x 512 size where each level
// is 2 px wide, therefore each thread group writes a single column and we can use the
// SV_GroupID semantic to get the group index = histogram level index
[numthreads(2, 256, 1)]
void main(uint idxInGroup : SV_GroupIndex, uint3 groupID : SV_GroupID, uint3 threadID : SV_DispatchThreadID) {
    // Compute the max value for this group so we can scale the histogram to look better
    // This is done only by the first thread in each group
    // We could do this for example by another CS pass on the histogram buffer but this allows us to
    // show groupshared storage and thread synchronization.
    if (idxInGroup == 0) {
        // SV_GroupIndex gives us indes of the thread in group
        for (int i = 0; i < 256; ++i) {
            maxValues = max(maxValues, histogram[i]);
        }
    }
    // All threads in each group need to reach this point before any is allowed to continue
    GroupMemoryBarrierWithGroupSync();

    uint count = 0;
    uint value = 0;
    uint maxVal = 0;
    float4 fillCol = float4(0, 0, 0, 1);

    // Calculate what to show in this column of the histogram
    [unroll]
    for (int i = 0; i < 4; ++i) {
        if (HistMask.x & (1 << i)) {
            value += histogram[groupID.x][i];
            maxVal = max(maxVal, maxValues[i]);
            fillCol.rgb += colors[i].rgb;
            ++count;
        }
    }
    // Divide by the number of values so we do not overflow the histogram
    value /= count;

    // All bits are set - use black
    if ((HistMask.x & 7) == 7) {
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
