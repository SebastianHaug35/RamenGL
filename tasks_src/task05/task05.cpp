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

static void AddTriangle(std::vector<Vertex>& vertices,
                        const Vec3f&         a,
                        const Vec3f&         b,
                        const Vec3f&         c,
                        const Vec3f&         normal)
{
    vertices.push_back(Vertex{ .position = a, .normal = normal });
    vertices.push_back(Vertex{ .position = b, .normal = normal });
    vertices.push_back(Vertex{ .position = c, .normal = normal });
}

static std::vector<Vertex> CreateInwardCube()
{
    std::vector<Vertex> vertices{};
    vertices.reserve(36);

    const float halfExtent = 1.0f;

    const Vec3f p000{ -halfExtent, -halfExtent, -halfExtent };
    const Vec3f p001{ -halfExtent, -halfExtent, halfExtent };
    const Vec3f p010{ -halfExtent, halfExtent, -halfExtent };
    const Vec3f p011{ -halfExtent, halfExtent, halfExtent };
    const Vec3f p100{ halfExtent, -halfExtent, -halfExtent };
    const Vec3f p101{ halfExtent, -halfExtent, halfExtent };
    const Vec3f p110{ halfExtent, halfExtent, -halfExtent };
    const Vec3f p111{ halfExtent, halfExtent, halfExtent };

    AddTriangle(vertices, p001, p011, p111, Vec3f{ 0.0f, 0.0f, -1.0f });
    AddTriangle(vertices, p001, p111, p101, Vec3f{ 0.0f, 0.0f, -1.0f });

    AddTriangle(vertices, p100, p110, p010, Vec3f{ 0.0f, 0.0f, 1.0f });
    AddTriangle(vertices, p100, p010, p000, Vec3f{ 0.0f, 0.0f, 1.0f });

    AddTriangle(vertices, p000, p010, p011, Vec3f{ 1.0f, 0.0f, 0.0f });
    AddTriangle(vertices, p000, p011, p001, Vec3f{ 1.0f, 0.0f, 0.0f });

    AddTriangle(vertices, p101, p111, p110, Vec3f{ -1.0f, 0.0f, 0.0f });
    AddTriangle(vertices, p101, p110, p100, Vec3f{ -1.0f, 0.0f, 0.0f });

    AddTriangle(vertices, p010, p110, p111, Vec3f{ 0.0f, -1.0f, 0.0f });
    AddTriangle(vertices, p010, p111, p011, Vec3f{ 0.0f, -1.0f, 0.0f });

    AddTriangle(vertices, p000, p001, p101, Vec3f{ 0.0f, 1.0f, 0.0f });
    AddTriangle(vertices, p000, p101, p100, Vec3f{ 0.0f, 1.0f, 0.0f });

    return vertices;
}

int main(int argc, char** argv)
{
    Filesystem* pFS = Filesystem::Init(argc, argv);

    Ramen* pRamen = Ramen::Instance();
    pRamen->Init("Task 05 - Cubemapping", 800, 600);

    /* Load shaders. */
    Shader shader{};
    if ( !shader.Load("shaders/task05.vert", "shaders/task05.frag") )
    {
        fprintf(stderr, "Could not load shader.\n");
    }

    Shader reflectShader{};
    if ( !reflectShader.Load("shaders/task05_reflect.vert", "shaders/task05_reflect.frag") )
    {
        fprintf(stderr, "Could not load reflection shader.\n");
    }

    /* Create camera */
    Camera camera(Vec3f{ 0.0f, 0.0f, 5.0f });
    camera.RotateAroundSide(0.0f);

    std::vector<Vertex> cubemapVertices = CreateInwardCube();

    Model reflectiveModel{};
    if ( !reflectiveModel.Load("models/teapot.obj") )
    {
        fprintf(stderr, "Could not load reflective model.\n");
    }

    Image cubemapPosX{};
    Image cubemapNegX{};
    Image cubemapPosY{};
    Image cubemapNegY{};
    Image cubemapPosZ{};
    Image cubemapNegZ{};

    if ( !cubemapPosX.Load("textures/posx.jpg") || !cubemapNegX.Load("textures/negx.jpg")
         || !cubemapPosY.Load("textures/posy.jpg") || !cubemapNegY.Load("textures/negy.jpg")
         || !cubemapPosZ.Load("textures/posz.jpg") || !cubemapNegZ.Load("textures/negz.jpg") )
    {
        fprintf(stderr, "Could not load cubemap textures.\n");
    }

    GLuint cubemapHandle = 0;
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &cubemapHandle);
    glTextureParameteri(cubemapHandle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(cubemapHandle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(cubemapHandle, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTextureParameteri(cubemapHandle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(cubemapHandle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureStorage2D(
        cubemapHandle, 1, GL_RGBA8, cubemapPosX.GetWidth(), cubemapPosX.GetHeight());

    glTextureSubImage3D(cubemapHandle,
                        0,
                        0,
                        0,
                        0,
                        cubemapPosX.GetWidth(),
                        cubemapPosX.GetHeight(),
                        1,
                        GL_RGBA,
                        GL_UNSIGNED_BYTE,
                        cubemapPosX.Data());
    glTextureSubImage3D(cubemapHandle,
                        0,
                        0,
                        0,
                        1,
                        cubemapNegX.GetWidth(),
                        cubemapNegX.GetHeight(),
                        1,
                        GL_RGBA,
                        GL_UNSIGNED_BYTE,
                        cubemapNegX.Data());
    glTextureSubImage3D(cubemapHandle,
                        0,
                        0,
                        0,
                        2,
                        cubemapPosY.GetWidth(),
                        cubemapPosY.GetHeight(),
                        1,
                        GL_RGBA,
                        GL_UNSIGNED_BYTE,
                        cubemapPosY.Data());
    glTextureSubImage3D(cubemapHandle,
                        0,
                        0,
                        0,
                        3,
                        cubemapNegY.GetWidth(),
                        cubemapNegY.GetHeight(),
                        1,
                        GL_RGBA,
                        GL_UNSIGNED_BYTE,
                        cubemapNegY.Data());
    glTextureSubImage3D(cubemapHandle,
                        0,
                        0,
                        0,
                        4,
                        cubemapPosZ.GetWidth(),
                        cubemapPosZ.GetHeight(),
                        1,
                        GL_RGBA,
                        GL_UNSIGNED_BYTE,
                        cubemapPosZ.Data());
    glTextureSubImage3D(cubemapHandle,
                        0,
                        0,
                        0,
                        5,
                        cubemapNegZ.GetWidth(),
                        cubemapNegZ.GetHeight(),
                        1,
                        GL_RGBA,
                        GL_UNSIGNED_BYTE,
                        cubemapNegZ.Data());

    /* Model mat*/
    Mat4f modelMat = Translate(Vec3f{ 0.0f, -1.0f, 0.0f }) * Scale(Vec3f{ 0.25f, 0.25f, 0.25f });
    Mat4f escapeSkyboxMat = Scale(Vec3f{ 20.0f, 20.0f, 20.0f });

    GLuint cubemapVAO = 0;
    glCreateVertexArrays(1, &cubemapVAO);
    GLuint cubemapVBO = 0;
    glCreateBuffers(1, &cubemapVBO);
    glNamedBufferData(
        cubemapVBO, cubemapVertices.size() * sizeof(Vertex), cubemapVertices.data(), GL_STATIC_DRAW);
    glVertexArrayVertexBuffer(cubemapVAO, 0, cubemapVBO, 0, sizeof(Vertex));

    glVertexArrayAttribFormat(cubemapVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glEnableVertexArrayAttrib(cubemapVAO, 0);
    glVertexArrayAttribBinding(cubemapVAO, 0, 0);

    glVertexArrayAttribFormat(cubemapVAO, 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float));
    glEnableVertexArrayAttrib(cubemapVAO, 1);
    glVertexArrayAttribBinding(cubemapVAO, 1, 0);

    GLuint modelVAO = 0;
    glCreateVertexArrays(1, &modelVAO);
    GLuint modelVBO = 0;
    glCreateBuffers(1, &modelVBO);
    glNamedBufferData(modelVBO,
                      reflectiveModel.GetVertices().size() * sizeof(Vertex),
                      reflectiveModel.GetVertices().data(),
                      GL_STATIC_DRAW);
    glVertexArrayVertexBuffer(modelVAO, 0, modelVBO, 0, sizeof(Vertex));

    glVertexArrayAttribFormat(modelVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glEnableVertexArrayAttrib(modelVAO, 0);
    glVertexArrayAttribBinding(modelVAO, 0, 0);

    glVertexArrayAttribFormat(modelVAO, 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float));
    glEnableVertexArrayAttrib(modelVAO, 1);
    glVertexArrayAttribBinding(modelVAO, 1, 0);

    /* Some global GL states */
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    // glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    bool fpsSkyboxMode = false;

    /* Main loop */
    bool isRunning = true;
    SDL_GL_SetSwapInterval(1); /* 1 = VSync enabled; 0 = VSync disabled */
    Uint64 ticksPerSecond = SDL_GetPerformanceFrequency();
    Uint64 startCounter   = SDL_GetPerformanceCounter();
    Uint64 endCounter     = SDL_GetPerformanceCounter();
    while ( isRunning )
    {
        double ticksPerFrame = (double)endCounter - (double)startCounter;
        double msPerFrame    = (ticksPerFrame / (double)ticksPerSecond) * 1000.0;
        float  deltaSeconds  = (float)(msPerFrame / 1000.0);
        startCounter         = SDL_GetPerformanceCounter();

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

                default:
                {
                }
                }
            }
        }

        const bool* keyboardState = SDL_GetKeyboardState(nullptr);
        float       moveSpeed     = 3.0f * deltaSeconds;
        float       rotateSpeed   = 90.0f * deltaSeconds;

        if ( keyboardState[ SDL_SCANCODE_W ] )
        {
            camera.MoveForward(moveSpeed);
        }
        if ( keyboardState[ SDL_SCANCODE_S ] )
        {
            camera.MoveForward(-moveSpeed);
        }
        if ( keyboardState[ SDL_SCANCODE_A ] )
        {
            camera.MoveRight(-moveSpeed);
        }
        if ( keyboardState[ SDL_SCANCODE_D ] )
        {
            camera.MoveRight(moveSpeed);
        }
        if ( keyboardState[ SDL_SCANCODE_Q ] )
        {
            camera.MoveUp(moveSpeed);
        }
        if ( keyboardState[ SDL_SCANCODE_E ] )
        {
            camera.MoveUp(-moveSpeed);
        }

        if ( keyboardState[ SDL_SCANCODE_LEFT ] )
        {
            camera.Yaw(rotateSpeed);
        }
        if ( keyboardState[ SDL_SCANCODE_RIGHT ] )
        {
            camera.Yaw(-rotateSpeed);
        }
        if ( keyboardState[ SDL_SCANCODE_UP ] )
        {
            camera.Pitch(rotateSpeed);
        }
        if ( keyboardState[ SDL_SCANCODE_DOWN ] )
        {
            camera.Pitch(-rotateSpeed);
        }

        /* Query new frame dimensions */
        int windowWidth, windowHeight;
        SDL_GetWindowSize(pRamen->GetWindow(), &windowWidth, &windowHeight);

        /* Adjust viewport and perspective projection accordingly. */
        glViewport(0, 0, windowWidth, windowHeight);

        /* View mat */
        Mat4f viewMat = LookAt(
            camera.GetPosition(), camera.GetPosition() + camera.GetForward(), camera.GetUp()); // Mat4f::Identity();

        Mat4f skyboxModelMat = fpsSkyboxMode ? Translate(camera.GetPosition()) * Scale(Vec3f{ 20.0f, 20.0f, 20.0f })
                                             : escapeSkyboxMat;

        /* Projection mat */
        float aspect  = (float)windowWidth / (float)windowHeight;
        Mat4f projMat = PerspectiveProjection(TO_RAD(60.0f), aspect, 0.01f, 500.0f);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Cubemap settings");

        ImGui::Text("5.0 Setup erfolgreich");
        ImGui::Text("5.1.0 Innenwuerfel aktiv");
        ImGui::Text("Vertices: %d", (int)cubemapVertices.size());
        ImGui::Separator();
        ImGui::Text("5.1.1 Cubemap-Textur auf GPU angelegt");
        ImGui::Text("Faces: +X -X +Y -Y +Z -Z");
        ImGui::Text("Cubemap size: %d x %d", cubemapPosX.GetWidth(), cubemapPosX.GetHeight());
        ImGui::Separator();
        ImGui::Text("5.1.2 Cubemap-Sampling aktiv");
        ImGui::Text("5.1.3 Kamera: W/A/S/D, Q/E, Pfeiltasten");
        ImGui::Text("5.2 Reflexionsmodell: teapot.obj");
        ImGui::Checkbox("5.3 FPS Skybox Mode", &fpsSkyboxMode);
        ImGui::Text("Skybox-Modus: %s", fpsSkyboxMode ? "FPS" : "Escape");
        ImGui::Text("Camera pos: %.2f %.2f %.2f",
                camera.GetPosition().x,
                camera.GetPosition().y,
                camera.GetPosition().z);

        ImGui::End();

        /* ImGUI Rendering */
        ImGui::Render();

        /* Rendering */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);

        glDepthMask(GL_FALSE);
        shader.Use();
        glBindVertexArray(cubemapVAO);
        glUniformMatrix4fv(0, 1, GL_FALSE, skyboxModelMat.Data());
        glUniformMatrix4fv(1, 1, GL_FALSE, viewMat.Data());
        glUniformMatrix4fv(2, 1, GL_FALSE, projMat.Data());
        glBindTextureUnit(0, cubemapHandle);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)cubemapVertices.size());
        glDepthMask(GL_TRUE);

        reflectShader.Use();
        glBindVertexArray(modelVAO);
        glUniformMatrix4fv(0, 1, GL_FALSE, modelMat.Data());
        glUniformMatrix4fv(1, 1, GL_FALSE, viewMat.Data());
        glUniformMatrix4fv(2, 1, GL_FALSE, projMat.Data());
        glUniform3fv(3, 1, camera.GetPosition().Data());
        glBindTextureUnit(0, cubemapHandle);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)reflectiveModel.NumVertices());

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(pRamen->GetWindow());

        endCounter = SDL_GetPerformanceCounter();
    }

    /* GL Resources shutdown. */
    shader.Delete();
    reflectShader.Delete();
    glDeleteTextures(1, &cubemapHandle);
    glDeleteBuffers(1, &cubemapVBO);
    glDeleteVertexArrays(1, &cubemapVAO);
    glDeleteBuffers(1, &modelVBO);
    glDeleteVertexArrays(1, &modelVAO);

    /* Ramen Shutdown */
    pRamen->Shutdown();

    /* Filesystem deinit */
    PHYSFS_deinit();

    return 0;
}
