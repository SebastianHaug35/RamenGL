#version 460

layout (location = 0) in vec3 in_position;

/* NOTE:
   One could use 'glGetUniformLocation' on CPU-side instead
   of fixed location = ... qualifiers.
   But this is not recommended anymore.
   @See: OpenGL Superbible 7, page 156.
*/
layout (location = 0) uniform mat4 u_ModelMat;
layout (location = 1) uniform mat4 u_ViewMat;
layout (location = 2) uniform mat4 u_ProjMat;

void main()
{
    vec4 position = u_ProjMat * u_ViewMat * u_ModelMat * vec4(in_position, 1.0f);
    gl_Position = position;
}
