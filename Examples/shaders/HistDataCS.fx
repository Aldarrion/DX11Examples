#include "Util.h"

cbuffer ConstantBuffer : register(b0) {
    uint4 Dimensions; // width, height, levels, XXX
}

Texture2D srcImage : register(t0);

RWStructuredBuffer<uint4> histogram : register(u0);

[numthreads(8, 8, 1)]
void main(uint3 threadID : SV_DispatchThreadID) {
    // SV_DispatchThreadID is one of several semantics available for compute shaders.
    // This one gives us global indices of the thread, regardles of groups.
    uint2 texPos = threadID.xy;

    // Normally we would not have to do this if since reads/writes outisde
    // texture/UAV bounds are well defined (read -> return 0, write -> nop)
    // But here we would get 0 from the texture and then bias the histogram level 0
    if (texPos.x >= Dimensions.x || texPos.y >= Dimensions.y)
        return;

    uint colorLevels = Dimensions.z;

    // Do not sample use Load (operator[]) for direct pixel access
    // Since the texture is SRGB (has such view in C++) the color we get here is in liear space
    // But we want to work with SRGB colors for better precision and display -> we convert it
    float4 pixel = colToSRGB(srcImage[texPos]);
    uint4 pixelQuantized = floor(pixel * colorLevels);

    // UAVs are accessable for reading/writing from multiple threads if the threads
    // write to different places. Here, however, we need to read a value and add one to it
    // but some other thread may do the same thing at the same time -> we must use atomic increment.
    [unroll]
    for (int i = 0; i < 4; ++i) {
        InterlockedAdd(histogram[pixelQuantized[i]][i], 1);
        InterlockedMax(histogram[256][i], histogram[pixelQuantized[i]][i]);
    }
}
