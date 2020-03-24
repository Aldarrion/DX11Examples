#include "Util.h"

cbuffer ConstantBuffer : register(b0) {
    uint4 Dimensions; // width, height, levels, XXX
}

// The source image from which the histogram will be calculated
Texture2D srcImage : register(t0);

// UAV buffer where the histogram values will be stored
RWStructuredBuffer<uint4> histogram : register(u0);

// numthreads specifies how many threads per group there will be
// the amount of groups spawned is specified from C++ Dispatch call
// The group sizes are limited.
// In cs_4_x we can have Z only 1 and total count (x * y * z) must be at most 768.
// In cs_5_0 Z can go up to 64 and total count must be at most 1024.
// Here the total count is 16 * 16 = 256 and Z is 1
[numthreads(16, 16, 1)]
void main(uint3 threadID : SV_DispatchThreadID) {
    // SV_DispatchThreadID is one of several semantics available for compute shaders.
    // This one gives us global indices of the thread, regardles of groups.
    uint2 texPos = threadID.xy;

    // Normally we would not have to do this if since reads/writes outisde
    // texture/UAV bounds are well defined (read -> return 0, write -> nop)
    // But here we would get 0 from the texture and then bias the histogram level 0
    if (texPos.x >= Dimensions.x || texPos.y >= Dimensions.y)
        return;

    // Do not sample use Load (operator[]) for direct pixel access
    // Since the texture is SRGB (has such view in C++) the color we get here is in liear space
    // But we want to work with SRGB colors for better precision and display -> we convert it
    float4 pixel = colToSRGB(srcImage[texPos]);

    // Quantize the color value to one of `colorLevels` bucketes
    uint colorLevels = Dimensions.z;
    uint4 pixelQuantized = floor(pixel * colorLevels);

    // UAVs are accessable for reading/writing from multiple threads if the threads
    // write to different places. Here, however, we need to read a value and add one to it
    // but some other thread may do the same thing at the same time -> we must use atomic increment.
    [unroll]
    for (int i = 0; i < 4; ++i) {
        InterlockedAdd(histogram[pixelQuantized[i]][i], 1);
    }
}
