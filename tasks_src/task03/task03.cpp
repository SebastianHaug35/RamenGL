#include <glad/glad.h>

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <vector>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_impl_sdl3.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <physfs.h>

#include "../ramen/ramen.h"
#include "../ramen/rgl_camera.h"
#include "../ramen/rgl_defines.h"
#include "../ramen/rgl_filesystem.h"
#include "../ramen/rgl_image.h"
#include "../ramen/rgl_math.h"
#include "../ramen/rgl_model.h"
#include "../ramen/rgl_shader.h"
#include "../ramen/rgl_utils.h"

/* Helper to append one triangle with a shared flat normal and a single color.
 * We use this for planar surfaces where every point on the triangle should
 * have the same normal, for example cuboid faces or cylinder caps.
 */
static void AddTriangle(std::vector<Vertex>& vertices,
                        const Vec3f&         a,
                        const Vec3f&         b,
                        const Vec3f&         c,
                        const Vec3f&         normal,
                        const Vec3f&         color)
{
    vertices.push_back(Vertex{ .position = a, .normal = normal, .color = color });
    vertices.push_back(Vertex{ .position = b, .normal = normal, .color = color });
    vertices.push_back(Vertex{ .position = c, .normal = normal, .color = color });
}

/* Helper for smooth shading.
 * On a sphere, the direction from the center to the surface point is already
 * the outward normal, so Normalize(position) gives the correct vertex normal.
 */
static void AddSmoothTriangle(std::vector<Vertex>& vertices,
                              const Vec3f&         a,
                              const Vec3f&         b,
                              const Vec3f&         c,
                              const Vec3f&         color)
{
    vertices.push_back(Vertex{ .position = a, .normal = Normalize(a), .color = color });
    vertices.push_back(Vertex{ .position = b, .normal = Normalize(b), .color = color });
    vertices.push_back(Vertex{ .position = c, .normal = Normalize(c), .color = color });
}

/* Creates a cuboid centered at the origin.
 * Every face gets its own constant normal so later lighting shows hard edges.
 */
static std::vector<Vertex> CreateCuboid(const Vec3f& color)
{
    std::vector<Vertex> vertices{};
    vertices.reserve(36);

    const float hx = 0.5f;
    const float hy = 0.5f;
    const float hz = 0.5f;

    const Vec3f p000{ -hx, -hy, -hz };
    const Vec3f p001{ -hx, -hy, hz };
    const Vec3f p010{ -hx, hy, -hz };
    const Vec3f p011{ -hx, hy, hz };
    const Vec3f p100{ hx, -hy, -hz };
    const Vec3f p101{ hx, -hy, hz };
    const Vec3f p110{ hx, hy, -hz };
    const Vec3f p111{ hx, hy, hz };

    AddTriangle(vertices, p001, p101, p111, Vec3f{ 0.0f, 0.0f, 1.0f }, color);
    AddTriangle(vertices, p001, p111, p011, Vec3f{ 0.0f, 0.0f, 1.0f }, color);

    AddTriangle(vertices, p100, p000, p010, Vec3f{ 0.0f, 0.0f, -1.0f }, color);
    AddTriangle(vertices, p100, p010, p110, Vec3f{ 0.0f, 0.0f, -1.0f }, color);

    AddTriangle(vertices, p000, p001, p011, Vec3f{ -1.0f, 0.0f, 0.0f }, color);
    AddTriangle(vertices, p000, p011, p010, Vec3f{ -1.0f, 0.0f, 0.0f }, color);

    AddTriangle(vertices, p101, p100, p110, Vec3f{ 1.0f, 0.0f, 0.0f }, color);
    AddTriangle(vertices, p101, p110, p111, Vec3f{ 1.0f, 0.0f, 0.0f }, color);

    AddTriangle(vertices, p010, p011, p111, Vec3f{ 0.0f, 1.0f, 0.0f }, color);
    AddTriangle(vertices, p010, p111, p110, Vec3f{ 0.0f, 1.0f, 0.0f }, color);

    AddTriangle(vertices, p000, p100, p101, Vec3f{ 0.0f, -1.0f, 0.0f }, color);
    AddTriangle(vertices, p000, p101, p001, Vec3f{ 0.0f, -1.0f, 0.0f }, color);

    return vertices;
}

/* Creates a cylinder centered at the origin.
 * The mantle gets radial normals for smooth lighting; top and bottom use
 * constant normals because those surfaces are flat discs.
 */
static std::vector<Vertex> CreateCylinder(const Vec3f& color, int segments = 32)
{
    std::vector<Vertex> vertices{};
    vertices.reserve(segments * 12);

    const float radius       = 0.5f;
    const float halfHeight   = 0.5f;
    const float twoPi        = 6.28318530718f;
    const Vec3f topCenter{ 0.0f, 0.0f, halfHeight };
    const Vec3f bottomCenter{ 0.0f, 0.0f, -halfHeight };
    const Vec3f topNormal{ 0.0f, 0.0f, 1.0f };
    const Vec3f bottomNormal{ 0.0f, 0.0f, -1.0f };

    for ( int i = 0; i < segments; i++ )
    {
        const float angle0 = (twoPi * (float)i) / (float)segments;
        const float angle1 = (twoPi * (float)(i + 1)) / (float)segments;

        const Vec3f ring0{ cosf(angle0) * radius, sinf(angle0) * radius, 0.0f };
        const Vec3f ring1{ cosf(angle1) * radius, sinf(angle1) * radius, 0.0f };

        const Vec3f top0 = ring0 + Vec3f{ 0.0f, 0.0f, halfHeight };
        const Vec3f top1 = ring1 + Vec3f{ 0.0f, 0.0f, halfHeight };
        const Vec3f bottom0 = ring0 + Vec3f{ 0.0f, 0.0f, -halfHeight };
        const Vec3f bottom1 = ring1 + Vec3f{ 0.0f, 0.0f, -halfHeight };

        const Vec3f sideNormal0 = Normalize(Vec3f{ ring0.x, ring0.y, 0.0f });
        const Vec3f sideNormal1 = Normalize(Vec3f{ ring1.x, ring1.y, 0.0f });

        vertices.push_back(Vertex{ .position = bottom0, .normal = sideNormal0, .color = color });
        vertices.push_back(Vertex{ .position = bottom1, .normal = sideNormal1, .color = color });
        vertices.push_back(Vertex{ .position = top1, .normal = sideNormal1, .color = color });

        vertices.push_back(Vertex{ .position = bottom0, .normal = sideNormal0, .color = color });
        vertices.push_back(Vertex{ .position = top1, .normal = sideNormal1, .color = color });
        vertices.push_back(Vertex{ .position = top0, .normal = sideNormal0, .color = color });

        AddTriangle(vertices, topCenter, top0, top1, topNormal, color);
        AddTriangle(vertices, bottomCenter, bottom1, bottom0, bottomNormal, color);
    }

    return vertices;
}

/* Converts spherical coordinates into a 3D point.
 * This isolates the math so the sphere-generation loop stays readable.
 */
static Vec3f SpherePoint(float radius, float theta, float phi)
{
    const float sinPhi = sinf(phi);
    return Vec3f{ radius * cosf(theta) * sinPhi, radius * sinf(theta) * sinPhi, radius * cosf(phi) };
}

/* Creates a UV sphere centered at the origin.
 * The sphere is split into stacks and slices. Vertex normals are taken from
 * the normalized position so lighting will later look smooth.
 */
static std::vector<Vertex> CreateSphere(const Vec3f& color, int slices = 32, int stacks = 16)
{
    std::vector<Vertex> vertices{};
    vertices.reserve(slices * stacks * 6);

    const float radius = 0.5f;
    const float pi     = 3.14159265359f;
    const float twoPi  = 6.28318530718f;

    for ( int stack = 0; stack < stacks; stack++ )
    {
        const float phi0 = (pi * (float)stack) / (float)stacks;
        const float phi1 = (pi * (float)(stack + 1)) / (float)stacks;

        for ( int slice = 0; slice < slices; slice++ )
        {
            const float theta0 = (twoPi * (float)slice) / (float)slices;
            const float theta1 = (twoPi * (float)(slice + 1)) / (float)slices;

            const Vec3f p00 = SpherePoint(radius, theta0, phi0);
            const Vec3f p10 = SpherePoint(radius, theta1, phi0);
            const Vec3f p01 = SpherePoint(radius, theta0, phi1);
            const Vec3f p11 = SpherePoint(radius, theta1, phi1);

            if ( stack == 0 )
            {
                AddSmoothTriangle(vertices, p00, p01, p11, color);
            }
            else if ( stack == stacks - 1 )
            {
                AddSmoothTriangle(vertices, p00, p11, p10, color);
            }
            else
            {
                AddSmoothTriangle(vertices, p00, p11, p10, color);
                AddSmoothTriangle(vertices, p00, p01, p11, color);
            }
        }
    }

    return vertices;
}

/* Creates line segments that visualize sampled vertex normals.
 * Every pair of vertices forms one GL_LINES segment from the surface point
 * into the normal direction.
 */
static std::vector<Vertex> CreateNormalLines(const std::vector<Vertex>& source,
                                             const Vec3f&               color,
                                             float                      length   = 0.18f,
                                             int                        maxLines = 160)
{
    std::vector<Vertex> vertices{};
    if ( source.empty() )
    {
        return vertices;
    }

    int step = (int)source.size() / maxLines;
    if ( step < 1 )
    {
        step = 1;
    }

    for ( int i = 0; i < (int)source.size(); i += step )
    {
        const Vertex& v = source[ i ];
        const Vec3f  endPosition = v.position + Normalize(v.normal) * length;

        vertices.push_back(Vertex{ .position = v.position, .normal = v.normal, .color = color });
        vertices.push_back(Vertex{ .position = endPosition, .normal = v.normal, .color = color });
    }

    return vertices;
}

class MatrixStack
{
  public:
    MatrixStack()
    {
        m_stack.push_back(Mat4f::Identity());
    }

    void Push()
    {
        m_stack.push_back(m_stack.back());
    }

    void Pop()
    {
        if ( m_stack.size() > 1 )
        {
            m_stack.pop_back();
        }
    }

    Mat4f& Last()
    {
        return m_stack.back();
    }

    void Translate(const Vec3f& v)
    {
        m_stack.back() = m_stack.back() * ::Translate(v);
    }

    void Rotate(const Vec3f& axis, float angleDgr)
    {
        m_stack.back() = m_stack.back() * ::Rotate(axis, angleDgr);
    }

    void Scale(const Vec3f& v)
    {
        m_stack.back() = m_stack.back() * ::Scale(v);
    }

  private:
    std::vector<Mat4f> m_stack;
};

int main(int argc, char** argv)
{
    Filesystem* pFS = Filesystem::Init(argc, argv, "assets");

    Ramen* pRamen = Ramen::Instance();
    pRamen->Init("GUI", 800, 600);

    /* Load shaders. */
    Shader shader{};
    if ( !shader.Load("shaders/task03.vert", "shaders/task03.frag") )
    {
        fprintf(stderr, "Could not load shader.\n");
    }

    Camera camera(Vec3f{ 0.0f, 1.0f, 3.0f });
    camera.RotateAroundSide(-15.0f);


    struct MeshGpu
    {
        GLuint vbo;
        GLuint normalVbo;
        GLsizei vertexCount;
        GLsizei normalVertexCount;
        Mat4f modelMat;
    };

    /* Create all requested primitives. The crane scene below uses all three of them. */
    std::vector<Vertex> cuboidVertices      = CreateCuboid(Vec3f{ 0.9f, 0.7f, 0.2f });
    std::vector<Vertex> cylinderVertices    = CreateCylinder(Vec3f{ 0.2f, 0.7f, 0.9f });
    std::vector<Vertex> sphereVertices      = CreateSphere(Vec3f{ 0.8f, 0.3f, 0.3f });
    std::vector<Vertex> lightMarkerVertices = CreateSphere(Vec3f{ 1.0f, 1.0f, 1.0f }, 16, 8);

    const Vec3f debugNormalColor{ 0.2f, 1.0f, 0.25f };
    std::vector<Vertex> cuboidNormalLines   = CreateNormalLines(cuboidVertices, debugNormalColor, 0.20f, 80);
    std::vector<Vertex> cylinderNormalLines = CreateNormalLines(cylinderVertices, debugNormalColor, 0.20f, 120);
    std::vector<Vertex> sphereNormalLines   = CreateNormalLines(sphereVertices, debugNormalColor, 0.20f, 160);

    MeshGpu meshes[ 4 ] = {
        { 0,
          0,
          (GLsizei)cuboidVertices.size(),
          (GLsizei)cuboidNormalLines.size(),
          Mat4f::Identity() },
        { 0,
          0,
          (GLsizei)cylinderVertices.size(),
          (GLsizei)cylinderNormalLines.size(),
          Mat4f::Identity() },
        { 0,
          0,
          (GLsizei)sphereVertices.size(),
          (GLsizei)sphereNormalLines.size(),
          Mat4f::Identity() },
        { 0, 0, (GLsizei)lightMarkerVertices.size(), 0, Mat4f::Identity() },
    };

    glCreateBuffers(1, &meshes[ 0 ].vbo);
    glNamedBufferData(
        meshes[ 0 ].vbo, cuboidVertices.size() * sizeof(Vertex), cuboidVertices.data(), GL_STATIC_DRAW);
    glCreateBuffers(1, &meshes[ 1 ].vbo);
    glNamedBufferData(
        meshes[ 1 ].vbo, cylinderVertices.size() * sizeof(Vertex), cylinderVertices.data(), GL_STATIC_DRAW);
    glCreateBuffers(1, &meshes[ 2 ].vbo);
    glNamedBufferData(
        meshes[ 2 ].vbo, sphereVertices.size() * sizeof(Vertex), sphereVertices.data(), GL_STATIC_DRAW);
    glCreateBuffers(1, &meshes[ 3 ].vbo);
    glNamedBufferData(meshes[ 3 ].vbo,
                      lightMarkerVertices.size() * sizeof(Vertex),
                      lightMarkerVertices.data(),
                      GL_STATIC_DRAW);

    glCreateBuffers(1, &meshes[ 0 ].normalVbo);
    glNamedBufferData(
        meshes[ 0 ].normalVbo, cuboidNormalLines.size() * sizeof(Vertex), cuboidNormalLines.data(), GL_STATIC_DRAW);
    glCreateBuffers(1, &meshes[ 1 ].normalVbo);
    glNamedBufferData(meshes[ 1 ].normalVbo,
                      cylinderNormalLines.size() * sizeof(Vertex),
                      cylinderNormalLines.data(),
                      GL_STATIC_DRAW);
    glCreateBuffers(1, &meshes[ 2 ].normalVbo);
    glNamedBufferData(
        meshes[ 2 ].normalVbo, sphereNormalLines.size() * sizeof(Vertex), sphereNormalLines.data(), GL_STATIC_DRAW);

    /* VAO. */
    GLuint VAO;
    glCreateVertexArrays(1, &VAO);
    glVertexArrayVertexBuffer(VAO, 0, meshes[ 0 ].vbo, 0, sizeof(Vertex));
    /* Position */
    glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glEnableVertexArrayAttrib(VAO, 0);
    glVertexArrayAttribBinding(VAO, 0, 0);
    /* Normal */
    glVertexArrayAttribFormat(VAO, 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float));
    glEnableVertexArrayAttrib(VAO, 1);
    glVertexArrayAttribBinding(VAO, 1, 0);

    /* Color attribute.
     * Vertex is laid out as position, normal, color.
     * That means color starts after 6 floats and is read by shader location 2.
     */
    glVertexArrayAttribFormat(VAO, 2, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float));
    glEnableVertexArrayAttrib(VAO, 2);
    glVertexArrayAttribBinding(VAO, 2, 0);

    /* Some global GL states */
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    // glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    const Uint64 ticksPerSecond = SDL_GetPerformanceFrequency();
    Uint64       lastCounter    = SDL_GetPerformanceCounter();
    float        elapsedSeconds = 0.0f;
    bool         showNormalColors = false;
    bool         showNormalLines  = false;

    /* Main loop */
    bool isRunning = true;
    SDL_GL_SetSwapInterval(1); /* 1 = VSync enabled; 0 = VSync disabled */
    while ( isRunning )
    {
        const Uint64 currentCounter = SDL_GetPerformanceCounter();
        const double deltaSeconds   = (double)(currentCounter - lastCounter) / (double)ticksPerSecond;
        lastCounter                 = currentCounter;
        elapsedSeconds += (float)deltaSeconds;

        SDL_Event e;
        while ( SDL_PollEvent(&e) )
        {
            ImGui_ImplSDL3_ProcessEvent(&e);
            pRamen->ProcessInputEvent(e);

            if ( e.type == SDL_EVENT_QUIT )
            {
                isRunning = false;
            }

            if ( e.type == SDL_EVENT_KEY_DOWN )
            {
                switch ( e.key.key )
                {
                case SDLK_ESCAPE:
                {
                    isRunning = false;
                }
                break;

                case SDLK_1:
                {
                    showNormalColors = !showNormalColors;
                }
                break;

                case SDLK_2:
                {
                    showNormalLines = !showNormalLines;
                }
                break;

                default:
                {
                }
                }
            }
        }

        const bool* keyboardState = SDL_GetKeyboardState(nullptr);
        const bool  fastCamera    = keyboardState[ SDL_SCANCODE_LSHIFT ] || keyboardState[ SDL_SCANCODE_RSHIFT ];
        const float moveSpeed     = fastCamera ? 4.0f : 2.0f;
        const float rotateSpeed   = 75.0f;
        const float moveStep      = moveSpeed * (float)deltaSeconds;
        const float rotateStep    = rotateSpeed * (float)deltaSeconds;

        if ( keyboardState[ SDL_SCANCODE_W ] )
        {
            camera.MoveForward(moveStep);
        }
        if ( keyboardState[ SDL_SCANCODE_S ] )
        {
            camera.MoveForward(-moveStep);
        }
        if ( keyboardState[ SDL_SCANCODE_D ] )
        {
            camera.MoveRight(moveStep);
        }
        if ( keyboardState[ SDL_SCANCODE_A ] )
        {
            camera.MoveRight(-moveStep);
        }
        if ( keyboardState[ SDL_SCANCODE_E ] )
        {
            camera.MoveUp(moveStep);
        }
        if ( keyboardState[ SDL_SCANCODE_Q ] )
        {
            camera.MoveUp(-moveStep);
        }
        if ( keyboardState[ SDL_SCANCODE_LEFT ] )
        {
            camera.Yaw(-rotateStep);
        }
        if ( keyboardState[ SDL_SCANCODE_RIGHT ] )
        {
            camera.Yaw(rotateStep);
        }
        if ( keyboardState[ SDL_SCANCODE_UP ] )
        {
            camera.Pitch(rotateStep);
        }
        if ( keyboardState[ SDL_SCANCODE_DOWN ] )
        {
            camera.Pitch(-rotateStep);
        }
        if ( keyboardState[ SDL_SCANCODE_R ] )
        {
            camera.Roll(-rotateStep);
        }
        if ( keyboardState[ SDL_SCANCODE_F ] )
        {
            camera.Roll(rotateStep);
        }

        /* Query new frame dimensions */
        int windowWidth, windowHeight;
        SDL_GetWindowSize(pRamen->GetWindow(), &windowWidth, &windowHeight);

        /* Adjust viewport and perspective projection accordingly. */
        glViewport(0, 0, windowWidth, windowHeight);

        /* View mat */
        Mat4f viewMat = LookAt(
            camera.GetPosition(), camera.GetPosition() + camera.GetForward(), camera.GetUp()); // Mat4f::Identity();

        const float lightRadius = 2.6f;
        const float lightHeight = 1.4f;
        const Vec3f lightWorldPos{ cosf(elapsedSeconds) * lightRadius, lightHeight, sinf(elapsedSeconds) * lightRadius };
        Vec4f       lightViewPos4 = viewMat * Vec4f{ lightWorldPos, 1.0f };
        Vec3f       lightViewPos{ lightViewPos4.x, lightViewPos4.y, lightViewPos4.z };
        meshes[ 3 ].modelMat = Translate(lightWorldPos) * Scale(Vec3f{ 0.12f, 0.12f, 0.12f });

        /* Projection mat */
        float aspect  = (float)windowWidth / (float)windowHeight;
        Mat4f projMat = PerspectiveProjection(TO_RAD(60.0f), aspect, 0.01f, 500.0f);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Normalen Debug");
        ImGui::Text("3.4 Debugging der Normalenvektoren");
        ImGui::Checkbox("1 Normalen als Farben", &showNormalColors);
        ImGui::Checkbox("2 Normalenlinien", &showNormalLines);
        ImGui::Text("Hotkeys: 1 / 2");
        ImGui::End();

        ImGui::Begin("Kamera");
        ImGui::Text("3.6 Tastatursteuerung");
        ImGui::Text("W/S vor/zurueck, A/D links/rechts");
        ImGui::Text("Q/E runter/hoch, Shift schneller");
        ImGui::Text("Pfeile drehen, R/F rollen");
        ImGui::End();

        /* ImGUI Rendering */
        ImGui::Render();

        /* Rendering */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);

        shader.Use();
        glBindVertexArray(VAO);
        glUniformMatrix4fv(1, 1, GL_FALSE, viewMat.Data());
        glUniformMatrix4fv(2, 1, GL_FALSE, projMat.Data());
        glUniform3fv(3, 1, lightViewPos.Data());

        glUniform1i(4, 0);
        glUniform1i(5, showNormalColors ? 1 : 0);

        auto drawMesh = [&](int meshIndex, Mat4f modelMat) {
            glVertexArrayVertexBuffer(VAO, 0, meshes[ meshIndex ].vbo, 0, sizeof(Vertex));
            glUniformMatrix4fv(0, 1, GL_FALSE, modelMat.Data());
            glDrawArrays(GL_TRIANGLES, 0, meshes[ meshIndex ].vertexCount);
        };

        auto drawNormalLines = [&](int meshIndex, Mat4f modelMat) {
            if ( !showNormalLines )
            {
                return;
            }

            glUniform1i(4, 1);
            glUniform1i(5, 0);
            glVertexArrayVertexBuffer(VAO, 0, meshes[ meshIndex ].normalVbo, 0, sizeof(Vertex));
            glUniformMatrix4fv(0, 1, GL_FALSE, modelMat.Data());
            glDrawArrays(GL_LINES, 0, meshes[ meshIndex ].normalVertexCount);
            glUniform1i(4, 0);
            glUniform1i(5, showNormalColors ? 1 : 0);
        };

        const float craneYaw = sinf(elapsedSeconds * 0.7f) * 35.0f;

        MatrixStack craneStack{};
        craneStack.Translate(Vec3f{ 0.0f, -0.65f, 0.0f });

        craneStack.Push();
        craneStack.Scale(Vec3f{ 0.9f, 0.15f, 0.9f });
        drawMesh(0, craneStack.Last());
        drawNormalLines(0, craneStack.Last());
        craneStack.Pop();

        craneStack.Push();
        craneStack.Translate(Vec3f{ 0.0f, 0.8f, 0.0f });
        craneStack.Scale(Vec3f{ 0.18f, 1.6f, 0.18f });
        drawMesh(0, craneStack.Last());
        drawNormalLines(0, craneStack.Last());
        craneStack.Pop();

        craneStack.Push();
        craneStack.Translate(Vec3f{ 0.0f, 1.65f, 0.0f });
        craneStack.Rotate(RAMEN_WORLD_UP, craneYaw);

        craneStack.Push();
        craneStack.Translate(Vec3f{ 0.9f, 0.0f, 0.0f });
        craneStack.Scale(Vec3f{ 1.8f, 0.12f, 0.12f });
        drawMesh(0, craneStack.Last());
        drawNormalLines(0, craneStack.Last());
        craneStack.Pop();

        craneStack.Push();
        craneStack.Translate(Vec3f{ 1.75f, 0.0f, 0.0f });

        craneStack.Push();
        craneStack.Translate(Vec3f{ 0.0f, -0.35f, 0.0f });
        craneStack.Rotate(RAMEN_WORLD_RIGHT, 90.0f);
        craneStack.Scale(Vec3f{ 0.08f, 0.08f, 0.7f });
        drawMesh(1, craneStack.Last());
        drawNormalLines(1, craneStack.Last());
        craneStack.Pop();

        craneStack.Push();
        craneStack.Translate(Vec3f{ 0.0f, -0.78f, 0.0f });
        craneStack.Scale(Vec3f{ 0.32f, 0.32f, 0.32f });
        drawMesh(2, craneStack.Last());
        drawNormalLines(2, craneStack.Last());
        craneStack.Pop();

        craneStack.Pop();
        craneStack.Pop();

        glUniform1i(4, 1);
        glUniform1i(5, 0);

        glVertexArrayVertexBuffer(VAO, 0, meshes[ 3 ].vbo, 0, sizeof(Vertex));
        glUniformMatrix4fv(0, 1, GL_FALSE, meshes[ 3 ].modelMat.Data());
        glDrawArrays(GL_TRIANGLES, 0, meshes[ 3 ].vertexCount);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(pRamen->GetWindow());
    }

    /* GL Resources shutdown. */
    shader.Delete();
    glDeleteBuffers(1, &meshes[ 0 ].vbo);
    glDeleteBuffers(1, &meshes[ 1 ].vbo);
    glDeleteBuffers(1, &meshes[ 2 ].vbo);
    glDeleteBuffers(1, &meshes[ 3 ].vbo);
    glDeleteBuffers(1, &meshes[ 0 ].normalVbo);
    glDeleteBuffers(1, &meshes[ 1 ].normalVbo);
    glDeleteBuffers(1, &meshes[ 2 ].normalVbo);
    glDeleteVertexArrays(1, &VAO);

    /* Ramen Shutdown */
    pRamen->Shutdown();

    /* Filesystem deinit */
    PHYSFS_deinit();

    return 0;
}
