Texture2D gPosition : register(t0);
Texture2D gNormal : register(t1);
Texture2D texNoise : register(t2);

SamplerState noiseSampler : register(s0);

cbuffer ConstantBuffer : register(b0) {
    matrix projection;
    float4 samples[64];
}


struct VS_INPUT {
    float4 Pos : POSITION;
    float2 UV : TEXCOORD0;
};

struct PS_INPUT {
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};


// =============
// Vertex Shader
// =============
PS_INPUT VS(VS_INPUT input) {
    PS_INPUT output = (PS_INPUT)0;

    output.Pos = input.Pos;
    output.Tex = input.UV;

    return output;
}

// ============
// Pixel shader
// ============
float4 PS(PS_INPUT input) : SV_Target {
    int kernelSize = 64;
    float radius = 0.5;
    float bias = 0.025;
    float2 noiseScale = float2(1280.0 / 4.0, 720.0 / 4.0);
    
    float3 ret;
    // get input for SSAO algorithm
    float3 fragPos = gPosition.Sample(noiseSampler, input.Tex).xyz;
    float3 normal = normalize(gNormal.Sample(noiseSampler, input.Tex).xyz);
    float3 randomVec = normalize(texNoise.Sample(noiseSampler, input.Tex * noiseScale).xyz);
    // create TBN change-of-basis matrix: from tangent-space to view-space
    float3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    float3 bitangent = normalize(cross(tangent, normal));
    //float3 bitangent = normalize(cross(normal, tangent));
    ret = bitangent;
    //float3x3 TBN = float3x3(tangent, bitangent, normal);
    float3x3 TBN = float3x3(tangent, bitangent, normal);
    //TBN = transpose(TBN);
    // iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for (int i = 0; i < kernelSize; ++i) {
        // get sample position
        float3 s = mul(samples[i].xyz, TBN); // from tangent to view-space
        s = fragPos + s * radius;

        // project sample position (to sample texture) (to get position on screen/texture)
        float4 offset = float4(s, 1.0);
        offset = mul(offset, projection); // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.x = offset.x * 0.5 + 0.5;
        offset.y = offset.y * (-0.5) + 0.5;
        //offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0

        // get sample depth
        float sampleDepth = gPosition.Sample(noiseSampler, offset.xy).z; // get depth value of kernel sample

        // range check & accumulate
        //float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        float rangeCheck = abs(fragPos.z - sampleDepth) < radius ? 1.0 : 0.0;
        occlusion += (sampleDepth <= s.z - bias ? 1.0 : 0.0) * rangeCheck;
    }
    occlusion = 1.0 - (occlusion / kernelSize);
    //occlusion = 1 - occlusion;

    //return float4(ret.x, ret.y, ret.z, 1.0);
    return float4(occlusion, occlusion, occlusion, 1.0);
}
