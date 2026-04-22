#version 460

out vec4 outColor;

layout(location = 0) in vec3 in_Normal;
layout(location = 1) in vec3 in_ViewSpacePos;
layout(location = 2) in vec3 in_Color;

layout(location = 3) uniform vec3 u_LightViewPos;
layout(location = 4) uniform int u_Unlit;
layout(location = 5) uniform int u_DebugNormals;


void main()
{
    if ( u_Unlit == 1 )
    {
        outColor = vec4(in_Color, 1.0);
        return;
    }

    vec3 normal = normalize(in_Normal);
    if ( u_DebugNormals == 1 )
    {
        outColor = vec4(normal * 0.5 + 0.5, 1.0);
        return;
    }

    vec3 lightDir = normalize(u_LightViewPos - in_ViewSpacePos);

    float diffuse = max(dot(normal, lightDir), 0.0);
    vec3 ambient = 0.18 * in_Color;
    vec3 litColor = ambient + diffuse * in_Color;

    outColor = vec4(litColor, 1.0);
}
