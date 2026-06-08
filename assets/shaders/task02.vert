#version 460 // Importance: 9/10 - Declares GLSL version; needed for modern syntax/features. GLSL means OpenGL Shading Language. It is the small C-like language you write for GPU programs like vertex shaders and fragment shaders.

layout (location = 0) in vec3 in_position; // Importance: 10/10 - Per-vertex object-space position read from currently bound VAO attribute 0 (CPU configured via glVertexArrayAttribFormat + glBindVertexArray).

/* NOTE:
   One could use 'glGetUniformLocation' on CPU-side instead
   of fixed location = ... qualifiers.
   But this is not recommended anymore.
   @See: OpenGL Superbible 7, page 156.
*/
layout (location = 0) uniform mat4 u_ModelMat; // Importance: 9/10 - Operation 1: local/object -> world space (translate/rotate/scale the model in the scene).
layout (location = 1) uniform mat4 u_ViewMat; // Importance: 9/10 - Operation 2: world -> view space (inverse camera transform; puts camera at origin looking forward).
layout (location = 2) uniform mat4 u_ProjMat; // Importance: 10/10 - Operation 3: view -> clip space (perspective divide prepared; distant objects appear smaller).

void main() // Importance: 10/10 - Vertex shader entry point; runs once per input vertex.
{
    vec4 position = u_ProjMat * u_ViewMat * u_ModelMat * vec4(in_position, 1.0f); // Importance: 10/10 - Required pipeline operation: promote vec3 -> homogeneous vec4 (w=1), then apply Model, then View, then Projection (right-to-left multiplication) to get clip-space coordinates.
    gl_Position = position; // Importance: 10/10 - Writes clip-space output; fixed-function stages then do clipping and perspective divide (x/w, y/w, z/w) before rasterization.
}
