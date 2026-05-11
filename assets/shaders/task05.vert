#version 460


/* NOTE:
   One could use 'glGetUniformLocation' on CPU-side instead
   of fixed location = ... qualifiers.
   But this is not recommended anymore.
   @See: OpenGL Superbible 7, page 156.
*/
layout(location = 0) uniform mat4 u_ModelMat;
layout(location = 1) uniform mat4 u_ViewMat;
layout(location = 2) uniform mat4 u_ProjMat;


void main()
{
}
