#version 460

layout(location = 0) in vec3 in_WorldPos;
layout(location = 1) in vec3 in_WorldNormal;
layout(location = 2) in vec2 in_UV;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D u_GroundTexture;
layout(binding = 1) uniform sampler2D u_ShadowMap;

layout(location = 3) uniform vec3 u_CameraWorldPos;
layout(location = 4) uniform vec3 u_LightWorldPos;
layout(location = 5) uniform mat4 u_LightSpaceMat;
layout(location = 6) uniform vec3 u_BaseColor;
layout(location = 7) uniform int u_UseTexture;
layout(location = 8) uniform int u_EnableShadows;
layout(location = 9) uniform float u_TextureScale;
layout(location = 10) uniform int u_IsLightMarker;

float SampleShadow(vec3 worldNormal, vec3 lightDir)
{
    if ( u_EnableShadows == 0 )
    {
        return 1.0;
    }

    vec4 lightClip = u_LightSpaceMat * vec4(in_WorldPos, 1.0);
    vec3 lightNdc = lightClip.xyz / lightClip.w;
    vec3 shadowCoord = lightNdc * 0.5 + 0.5;

    if ( shadowCoord.x < 0.0 || shadowCoord.x > 1.0 || shadowCoord.y < 0.0 || shadowCoord.y > 1.0 || shadowCoord.z > 1.0 )
    {
        return 1.0;
    }

    float closestDepth = texture(u_ShadowMap, shadowCoord.xy).r;
    float currentDepth = shadowCoord.z;
    float bias = max(0.0025 * (1.0 - dot(worldNormal, lightDir)), 0.0006);

    return currentDepth > closestDepth + bias ? 0.35 : 1.0;
}

void main()
{
    if ( u_IsLightMarker == 1 )
    {
        outColor = vec4(u_BaseColor, 1.0);
        return;
    }

    vec3 albedo = u_BaseColor;
    if ( u_UseTexture == 1 )
    {
        albedo = texture(u_GroundTexture, in_UV * u_TextureScale).rgb;
    }

    vec3 normal = normalize(in_WorldNormal);
    vec3 lightDir = normalize(u_LightWorldPos - in_WorldPos);
    float diffuse = max(dot(normal, lightDir), 0.0);
    float shadow = SampleShadow(normal, lightDir);

    vec3 ambient = 0.22 * albedo;
    vec3 litColor = ambient + shadow * diffuse * albedo;

    outColor = vec4(litColor, 1.0);
}
