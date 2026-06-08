#version 460 // Importance: 9/10 - Selects GLSL version for this shader.

// Task 3.1 TODO: Receive and forward vertex color attribute to fragment shader.
//   Syntax breakdown for 'layout(location = X) in TYPE name;':
//   layout(location = X) -> tells the shader which VAO attribute slot to read from (matches glVertexArrayAttribFormat(..., X, ...))
//   in                   -> storage qualifier: this value comes IN from the CPU-side vertex buffer (read-only per vertex)
//   vec3                 -> data type: 3 floats (x,y,z)
//   in_Position/...      -> variable name (user-defined; 'in_' prefix is just a convention)
layout(location = 0) in vec3 in_Position; // Importance: 10/10 - [in] Per-vertex position read from VAO slot 0; object space.
layout(location = 1) in vec3 in_Normal;   // Importance: 10/10 - [in] Per-vertex normal read from VAO slot 1; object space, needed for lighting.
layout(location = 2) in vec3 in_Color;    // Importance: 9/10  - [in] Per-vertex color read from VAO slot 2 (Task 3.1).

// Task 3.5 TODO: Pass data to fragment shader for Lambert lighting calculations.
//   Syntax breakdown for 'layout(location = X) out TYPE name;':
//   out    -> storage qualifier: value is WRITTEN OUT of the vertex shader; the GPU interpolates it across the triangle
//             and delivers the interpolated value to the fragment shader as the matching 'in' variable
//   location = X -> must match the location = X on the 'in' declaration in the fragment shader
layout(location = 0) out vec3 out_Normal;       // Importance: 10/10 - [out->in_Normal in frag] View-space normal, interpolated across triangle. Normalenvektor wird pro Pixel gemischt. Deshalb normalisiert man im Fragment-Shader oft nochmal.
layout(location = 1) out vec3 out_ViewSpacePos; // Importance: 10/10 - [out->in_ViewSpacePos in frag] View-space position, needed for light direction vector. Position wird pro Pixel gemischt, damit Licht-Richtung pro Pixel korrekt berechnet wird.
layout(location = 2) out vec3 out_Color;        // Importance: 9/10  - [out->in_Color in frag] Vertex color, interpolated across triangle (Task 3.1). Farbe wird pro Pixel gemischt, daher Farbverlauf über die Fläche.

/* NOTE:
   One could use 'glGetUniformLocation' on CPU-side instead
   of fixed location = ... qualifiers.
   But this is not recommended anymore.
   @See: OpenGL Superbible 7, page 156.
*/
//   Syntax breakdown for 'layout(location = X) uniform TYPE name;':
//   uniform  -> storage qualifier: value is SET FROM THE CPU once per draw call (same value for every vertex/fragment)
//               in task03.cpp this is done with e.g. glProgramUniformMatrix4fv(prog, 0, 1, ...)
//   mat4     -> data type: 4x4 matrix of floats
//   u_ModelMat/... -> variable name (user-defined; 'u_' prefix = uniform, convention only)
//   location = 0   -> slot number used on CPU side as the second arg to glProgramUniform*(prog, 0, ...)
layout(location = 0) uniform mat4 u_ModelMat; // Importance: 9/10  - [uniform, set by CPU] Model matrix: object-space -> world-space. Easy: moves/rotates/scales the object to its place in the scene.
layout(location = 1) uniform mat4 u_ViewMat;  // Importance: 9/10  - [uniform, set by CPU] View matrix: world-space -> camera/view-space. Easy: places the whole world relative to the camera (like moving the camera view).
layout(location = 2) uniform mat4 u_ProjMat;  // Importance: 10/10 - [uniform, set by CPU] Projection matrix: view-space -> clip-space (perspective divide). Easy: makes far things look smaller and prepares coordinates for screen output.

void main() // Importance: 10/10 - Entry point; runs once per vertex.
{
    mat4 modelViewMat = u_ViewMat * u_ModelMat; // Importance: 9/10 - Combined MV matrix to transform into view space in one step.
    vec4 viewPos = modelViewMat * vec4(in_Position, 1.0); // Importance: 10/10 - Position in view space; needed for light direction in fragment shader.
    mat3 normalMat = transpose(inverse(mat3(modelViewMat))); // Importance: 10/10 - Task 3.5: Normal matrix corrects normals under non-uniform scale; without this, normals would be wrong after scaling.

    gl_Position = u_ProjMat * viewPos; // Importance: 10/10 - Final clip-space position for rasterization.

    out_ViewSpacePos = viewPos.xyz;               // Importance: 10/10 - Forward view-space position to fragment shader.
    out_Normal = normalize(normalMat * in_Normal); // Importance: 10/10 - Transform and normalize normal into view space.
    out_Color = in_Color;                          // Importance: 9/10 - Task 3.1: Forward vertex color to fragment shader.
}
