// SHININESS is defining "specularity" of surfaces in this shader
#define SHININESS 32

struct PointLight {
    float4 Position;
    float4 Color;
};

struct DirLight {
    float4 Direction;
    float4 Color;
};

struct SpotLight {
    float4 Position;
    float4 Direction;
    float4 Color;
    float4 InnerCone;
    float4 OuterCone;
};

float4 CalcDirLight(DirLight light, float3 normal, float4 fragColor, float3 viewDir, float shadow = 0.0, float4 specularity = 1.0, float exponent = SHININESS) {
    // ambient
    float ambientStrength = 0.25;
    float3 ambient = mul(ambientStrength, light.Color);
    
    // diffuse
    float3 lightDir = normalize(-light.Direction);
    float diff = max(dot(lightDir, normal), 0.0);
    float3 diffuse = diff * light.Color;

    // specular
    float specularStrength = 1.0;
    float3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), exponent);
    float3 specular = specularStrength * spec * light.Color * specularity;

    float4 finalColor = saturate(float4((ambient + (1.0 - shadow) * (diffuse + specular)), 1) * fragColor);
    finalColor.a = 1;

    return finalColor;
}

float4 CalcPointLight(
    PointLight light,
    float3 normal,
    float3 fragPos,
    float4 fragColor,
    float3 viewDir,
    float4 specularity = 1.0,
    float exponent = SHININESS,
    float ambientStr = 0.1
) {
    // ambient
    float ambientStrength = ambientStr;
    float3 ambient = mul(ambientStrength, light.Color);

    // diffuse
    float3 lightDir = normalize(light.Position - fragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    float3 diffuse = diff * light.Color;

    // specular
    float specularStrength = 1.0;
    float3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), exponent);
    float3 specular = specularStrength * spec * light.Color * specularity;

    float4 finalColor = saturate(float4((ambient + diffuse + specular), 1) * fragColor);
    finalColor.a = 1;

    return finalColor;
}

float4 CalcSpotLight(SpotLight light, float3 normal, float3 fragPos, float4 fragColor, float3 viewDir, float4 specularity = 1.0, float exponent = SHININESS) {
    // ambient
    float ambientStrength = 0.1;
    float3 ambient = mul(ambientStrength, light.Color);

    // diffuse
    float3 lightDir = normalize(light.Position - fragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    float3 diffuse = diff * light.Color;

    // specular
    float specularStrength = 1.0;
    float3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), exponent);
    float3 specular = specularStrength * spec * light.Color * specularity;

    float theta = dot(lightDir, normalize(light.Direction));
    float epsilon = light.InnerCone.x - light.OuterCone.x;
    float intensity = clamp((theta - light.OuterCone.x) / epsilon, 0.0, 1.0);

    ambient *= intensity;
    diffuse *= intensity;
    specular *= intensity;

    float4 finalColor = saturate(float4((ambient + diffuse + specular), 1) * fragColor);
    finalColor.a = 1;

    return finalColor;
}
