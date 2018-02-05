#define SHININESS 32

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
struct PointLight {
    float4 Position;
    float4 Color;
};

struct DirLight {
    float4 Direction;
    float4 Color;
};

cbuffer ConstantBuffer : register(b0) {
    matrix World;
    matrix View;
    matrix Projection;
    PointLight PointLights[2];
    DirLight DirLights[2];
    float3 ViewPos;
    int PointLightCount;
    int DirLightCount;
}


//--------------------------------------------------------------------------------------
struct VS_INPUT {
    float4 Pos : POSITION;
    float3 Norm : NORMAL;
    float4 Color : COLOR;
};

struct PS_INPUT {
    float4 Pos : SV_POSITION;
    float3 Norm : TEXCOORD0;
    float4 Color : COLOR;
    float3 FragPos : POSITION;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input) {
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = mul(input.Pos, World);
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);
    output.Norm = mul(float4(input.Norm, 1), World).xyz;
    output.Color = input.Color;
    output.FragPos = mul(input.Pos, World);
    //output.FragPos = input.Pos;

    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 CalcDirLight(DirLight light, float3 normal, float3 fragPos, float4 fragColor) {
    // ambient
    float ambientStrength = 0.1;
    float3 ambient = mul(ambientStrength, light.Color);
    
    // diffuse
    float3 lightDir = normalize(-light.Direction);
    float diff = max(dot(lightDir, normal), 0.0);
    float3 diffuse = diff * light.Color;

    // specular
    float specularStrength = 0.5;
    float3 viewDir = normalize(ViewPos - fragPos);
    float3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), SHININESS);
    float3 specular = specularStrength * spec * light.Color;

    float4 finalColor = saturate(float4((ambient + diffuse + specular), 1) * fragColor);
    finalColor.a = 1;

    return finalColor;
}

float4 CalcPointLight(PointLight light, float3 normal, float3 fragPos, float4 fragColor) {
    // ambient
    float ambientStrength = 0.1;
    float3 ambient = mul(ambientStrength, light.Color);

    // diffuse
    float3 lightDir = normalize(light.Position - fragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    float3 diffuse = diff * light.Color;

    // specular
    float specularStrength = 0.5;
    float3 viewDir = normalize(ViewPos - fragPos);
    float3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), SHININESS);
    float3 specular = specularStrength * spec * light.Color;

    float4 finalColor = saturate(float4((ambient + diffuse + specular), 1) * fragColor);
    finalColor.a = 1;

    return finalColor;
}

float4 PS(PS_INPUT input) : SV_Target {
    float3 normal = normalize(input.Norm);
    
    float4 finalColor = float4(0.0, 0.0, 0.0, 0.0);
    for (int i = 0; i < DirLightCount; ++i) {
        finalColor += CalcDirLight(DirLights[i], normal, input.FragPos, input.Color);
    }
    for (int i = 0; i < PointLightCount; ++i) {
        finalColor += CalcPointLight(PointLights[i], normal, input.FragPos, input.Color);
    }

    finalColor = saturate(finalColor);

    return finalColor;
}
