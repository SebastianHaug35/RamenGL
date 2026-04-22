#version 460

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec3 in_Color;

layout(location = 0) out vec3 out_Normal;
layout(location = 1) out vec3 out_ViewSpacePos;
layout(location = 2) out vec3 out_Color;

/* NOTE:
   One could use 'glGetUniformLocation' on CPU-side instead
   of fixed location = ... qualifiers.
   But this is not recommended anymore.
   @See: OpenGL Superbible 7, page 156.
*/
layout(location = 0) uniform mat4 u_ModelMat;
layout(location = 1) uniform mat4 u_ViewMat;
layout(location = 2) uniform mat4 u_ProjMat;

const vec3 lightPos = vec3(10.0f, 1.0f, 10.0f);
const vec3 ambientColor = vec3(1.0f, 0.95f, 0.0f);

void main()
{
    vec4 position = u_ProjMat * u_ViewMat * u_ModelMat * vec4(in_Position, 1.0f);
    gl_Position = position;
    out_Color = vec3(1.0f, 0.0f, 0.0f);
}
