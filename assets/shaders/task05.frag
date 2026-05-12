#version 460

out vec4 outColor;

layout(location = 0) in vec3 in_Direction;

layout(binding = 0) uniform samplerCube u_Cubemap;

void main()
{
    vec3 sampleDir = normalize(in_Direction);
    outColor = texture(u_Cubemap, sampleDir);
}
