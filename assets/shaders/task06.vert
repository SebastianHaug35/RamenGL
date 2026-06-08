#version 460

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec3 in_Color;
layout(location = 3) in vec3 in_UV;

layout(location = 0) out vec3 out_WorldPos;
layout(location = 1) out vec3 out_WorldNormal;
layout(location = 2) out vec2 out_UV;

layout(location = 0) uniform mat4 u_ModelMat;
layout(location = 1) uniform mat4 u_ViewMat;
layout(location = 2) uniform mat4 u_ProjMat;

void main()
{
    vec4 worldPos = u_ModelMat * vec4(in_Position, 1.0);
    mat3 normalMat = transpose(inverse(mat3(u_ModelMat)));

    out_WorldPos = worldPos.xyz;
    out_WorldNormal = normalize(normalMat * in_Normal);
    out_UV = in_UV.xy;

    gl_Position = u_ProjMat * u_ViewMat * worldPos;
}
