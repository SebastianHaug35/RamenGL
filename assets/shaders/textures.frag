#version 460

out vec4 outColor;

layout(location = 0) in vec3 in_Normal;
layout(location = 1) in vec3 in_ViewSpacePos;
layout(location = 2) in vec3 in_Color;
layout(location = 3) in vec3 in_UV;

layout(binding = 0) uniform sampler2D u_Texture;

void main()
{
    vec4 texColor = texture(u_Texture, in_UV.st);
    outColor = texColor;
}
