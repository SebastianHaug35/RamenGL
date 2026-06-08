#version 460

layout(location = 0) in vec3 out_WorldPos;
layout(location = 1) in vec3 out_WorldNormal;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform samplerCube u_Cubemap;
layout(location = 3) uniform vec3 u_CameraWorldPos;

void main()
{
    vec3 viewDir = normalize(out_WorldPos - u_CameraWorldPos);
    vec3 normal = normalize(out_WorldNormal);
    vec3 reflectionDir = reflect(viewDir, normal);

    outColor = texture(u_Cubemap, reflectionDir);
}