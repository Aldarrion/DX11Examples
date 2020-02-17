Texture2D gPosition : register(t0);
Texture2D gNormal : register(t1);
Texture2D texNoise : register(t2);

SamplerState pointSampler : register(s0);

cbuffer ConstantBuffer : register(b0) {
    matrix projection;
    float4 kernel[64];
    float4 screenResolution;
	int kernelSize;
	int randomRotation;
}


struct VS_INPUT {
    float4 Pos : POSITION;
    float2 UV : TEXCOORD0;
};

struct PS_INPUT {
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD0;
};


// =============
// Vertex Shader
// =============
PS_INPUT VS(VS_INPUT input) {
    PS_INPUT output = (PS_INPUT)0;

    output.Pos = input.Pos;
    output.UV = input.UV;

    return output;
}

// ============
// Pixel shader
// ============
float4 PS(PS_INPUT input) : SV_Target{
	//kernelSize = 64;
	float radius = 0.5;
	float bias = 0.025;

	// Control how fine will the noise be, the noise texture is 4x4, here we control how much
	// scaled down it will be. We use fine noise to avoid aliasing.
	float2 noiseScale = float2(screenResolution.x / 4.0, screenResolution.y / 4.0);

	// Retrieve data from the G-buffer
	float3 fragPos = gPosition.Sample(pointSampler, input.UV).xyz;
	float3 normal = normalize(gNormal.Sample(pointSampler, input.UV).xyz);
	float3 randomVec = normalize(texNoise.Sample(pointSampler, input.UV * noiseScale).xyz);

	// Create TBN transformation matrix from tangent space to view space
	// This matrix rotates the kernel by a random amount (randomVec) to 
	// avoid artifacts and introduce noise to the SSAO result instead
	float3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	float3 bitangent = normalize(cross(tangent, normal));
	float3x3 TBN = float3x3(tangent, bitangent, normal);

	if (randomRotation == 0) {
		TBN = float3x3(float3(1, 0, 0), float3(0, 1, 0), float3(0, 0, 1));
	}
    
    // Iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for (int i = 0; i < kernelSize; ++i) {
        // Get sample position
        float3 kernelPos = mul(kernel[i].xyz, TBN); // Transform from tangent to view space
        kernelPos = fragPos + kernelPos * radius; // Move kernel point to the fragment and scale

        // Transform kernel point position to texture coordinates to sample the G-Buffer
        float4 offset = float4(kernelPos, 1.0);
        offset = mul(offset, projection); // Transfrom to clip space
        offset.xyz /= offset.w; // Perspective divide (transform to NDC)
        // Transform from NDC to texture coordinates
        offset.x = offset.x * 0.5 + 0.5;
        offset.y = offset.y * (-0.5) + 0.5;

        // What is the depth of the actual geometry at the kernelPos
        // Sample this information from the G-buffer
        float sampleDepth = gPosition.Sample(pointSampler, offset.xy).z;

        // See whether the sample occludes our current fragmet
        // Range check is used to avoid fake occlusions, if the geometry (sample depth) is too far
        // it probably does not occlude
        float rangeCheck = abs(fragPos.z - sampleDepth) < radius ? 1.0 : 0.0;
        occlusion += (sampleDepth <= kernelPos.z - bias ? 1.0 : 0.0) * rangeCheck;
    }
    // Calculate average occlusion factor
    occlusion = 1.0 - (occlusion / kernelSize);

    return float4(occlusion, occlusion, occlusion, 1.0);
}
