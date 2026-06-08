#version 460 // Importance: 9/10 - Selects GLSL version for this shader.

out vec4 outColor; // Importance: 10/10 - Final output color written to framebuffer.

// Task 3.1 TODO: Receive vertex color from vertex shader.
//   'in' here means: value was interpolated by the rasterizer from the vertex shader's 'out' values.
//   location = X must match the location = X of the corresponding 'out' in task03.vert.
layout(location = 0) in vec3 in_Normal;       // Importance: 10/10 - [in from vert out_Normal]       Interpolated view-space normal; needed for Lambert dot product.
layout(location = 1) in vec3 in_ViewSpacePos; // Importance: 10/10 - [in from vert out_ViewSpacePos] Interpolated view-space position; used to compute light direction.
layout(location = 2) in vec3 in_Color;        // Importance: 9/10  - [in from vert out_Color]        Interpolated vertex color (Task 3.1).

// Task 3.5 TODO: Implement Lambert cosine law (diffuse point light shading).
//   All three are 'uniform': set once from CPU per frame via glProgramUniform*(prog, location, ...).
//   'vec3' = 3 floats, 'int' = single integer.
layout(location = 3) uniform vec3 u_LightViewPos; // Importance: 10/10 - [uniform vec3] Light position in view space; computed on CPU and passed in each frame.
layout(location = 4) uniform int  u_Unlit;         // Importance: 7/10  - [uniform int]  Debug flag (0/1): 1 = skip all lighting, output raw vertex color (used for normal lines and light marker).
layout(location = 5) uniform int  u_DebugNormals;  // Importance: 8/10  - [uniform int]  Debug flag (0/1): 1 = visualize normals as RGB colors (Task 3.4).

void main() // Importance: 10/10 - Fragment shader entry point; runs once per fragment.
{
    // Task 3.4 TODO: Debug mode 1 - render unlit (raw vertex color).
    if ( u_Unlit == 1 )
    {
        outColor = vec4(in_Color, 1.0); // Importance: 7/10 - Skip lighting; output raw color for normal-line geometry.
        return;
    }

    vec3 normal = normalize(in_Normal); // Importance: 10/10 - Re-normalize interpolated normal (interpolation shortens it).

    // Task 3.4 TODO: Debug mode 2 - visualize normals as RGB colors.
    if ( u_DebugNormals == 1 )
    {
        // normal is a vec3 in range [-1, 1] per component.
        // normal * 0.5 + 0.5 remaps each component to [0, 1], which is valid color range.
        // vec4(xyz, w) builds RGBA: xyz -> RGB from remapped normal, w -> alpha.
        // Here alpha is 1.0, so the fragment is fully opaque.
        outColor = vec4(normal * 0.5 + 0.5, 1.0);
        return;
    }

    // Task 3.5 TODO: Lambert cosine law - diffuse = max(dot(N, L), 0).
    vec3 lightDir = normalize(u_LightViewPos - in_ViewSpacePos); // Importance: 10/10 - Direction from fragment to light (both in view space).

    float diffuse = max(dot(normal, lightDir), 0.0); // Importance: 10/10 - Lambert term: cos(angle) clamped to >= 0 (no negative light).
    vec3 ambient = 0.18 * in_Color;                  // Importance: 8/10 - Constant ambient term so shadows are never fully black.
    vec3 litColor = ambient + diffuse * in_Color;     // Importance: 10/10 - Final lit color = ambient + diffuse contribution.

    outColor = vec4(litColor, 1.0); // Importance: 10/10 - Write final shaded color to framebuffer.
}
