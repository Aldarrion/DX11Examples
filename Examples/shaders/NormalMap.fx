#include "PhongLights.fx"

#define NUM_LIGHTS 2

Texture2D DiffuseTexture : register(t0);
Texture2D NormalMap : register(t1);

SamplerState DiffuseSampler : register(s0);

cbuffer ConstantBuffer : register(b0) {
    matrix World;
    matrix View;
    matrix Projection;
    matrix NormalMatrix;
    PointLight PointL[2];
    float3 ViewPos;
    int UseNormalMap;
}


struct VS_INPUT {
    float4 Pos : POSITION;
    float3 Norm : NORMAL;
    float4 Color : COLOR;
    float2 UV : TEXCOORD0;
    float3 Tangent : TANGENT;
    float3 Bitangent : BINORMAL;
};

struct PS_INPUT {
    float4 Pos : SV_POSITION;
    float3 Norm : TEXCOORD0;
    float3 FragPos : POSITION;
    float2 UV : TEXCOORD1;
    float3 TangentViewPos : TEXCOORD2;
    float3 TangentFragPos : TEXCOORD3;
    float3 TangentLightPos[NUM_LIGHTS] : TEXCOORD4;
};


// =============
// Vertex Shader
// =============
PS_INPUT VS(VS_INPUT input) {
    PS_INPUT output = (PS_INPUT)0;
    
    output.Pos = mul(input.Pos, World);
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);
    
    output.Norm = mul(float4(input.Norm, 1), NormalMatrix).xyz;
    output.FragPos = mul(input.Pos, World);
    output.UV = input.UV;

    // Transform TBN from model space to world space
    float3 N = normalize(output.Norm);
    float3 T = normalize(mul(NormalMatrix, float4(input.Tangent, 1)).xyz);

    // Perform Gram-Schmidt orthogonalization in case the transform did not preserve orthogonality
    T = normalize(T - dot(T, N) * N);
    float3 B = normalize(cross(T, N));
    
    /** 
     * The matrix is orthonormal -> we invert it by transposing it.
     * This matrix allows us to transform vertices from world space to tangent space, we could also use the 
     * inverse to transform from  tangent to world, but we would have to do it in each fragment
     * (for each sampled normal from the map), this way we do the transform only in vertex shader.
     */ 
    float3x3 worldToTangent = transpose(float3x3(T, B, N));

    // Transform all variables necessary for transform calculations to tangent space
    for (int i = 0; i < NUM_LIGHTS; ++i) {
        output.TangentLightPos[i] = mul(PointL[i].Position.xyz, worldToTangent);
    }
    output.TangentViewPos = mul(ViewPos, worldToTangent);
    output.TangentFragPos = mul(output.FragPos, worldToTangent);

    return output;
}


// ============
// Pixel shader
// ============
float4 PS(PS_INPUT input) : SV_Target {
    float4 diffuseColor = DiffuseTexture.Sample(DiffuseSampler, input.UV);
    
    float4 finalColor = float4(0, 0, 0, 0);

    if (UseNormalMap) { 
        // Normal mapping in tangent space
        /** 
         * Normal map filtering (sampling) should be performed better for more accurate results. Imagine stairs
         * where we have two normals - one having 0° angle and one having 90° angle, in case we would like
         * to filter this we would get around average angle = 45°. This would, however, produce some totally
         * unexpected specular reflections, since in the original stairs no light could produce them.
         * For our example with relatively smooth surface this filtering works well enough.
         */
        float3 normal = NormalMap.Sample(DiffuseSampler, input.UV).xyz;
        
        // Normal vector components are compressed to range [0, 1] when saved to texture - uncompress them to their full [-1, 1] range
        normal *= 2;
        normal -= float3(1, 1, 1);
        normal = normalize(normal);

        float3 viewDir = normalize(input.TangentViewPos - input.TangentFragPos);

        //DirLight tangentSun = SunLight;
        for (int i = 0; i < NUM_LIGHTS; ++i) {
            PointLight tangentLight = PointL[i];
            tangentLight.Position = float4(input.TangentLightPos[i], 1);
        
            // Perform tangent space light calculation
            finalColor += CalcPointLight(tangentLight, normal, input.TangentFragPos, diffuseColor, viewDir);
        }
    } else { 
        // Usual shading in world space
        float3 normal = normalize(input.Norm);
        float3 viewDir = normalize(ViewPos - input.FragPos);
        for (int i = 0; i < NUM_LIGHTS; ++i) {
            finalColor += CalcPointLight(PointL[i], normal, input.FragPos, diffuseColor, viewDir);
        }
    }
    
    finalColor.a = 1.0;
    finalColor = saturate(finalColor);

    return finalColor;
}
