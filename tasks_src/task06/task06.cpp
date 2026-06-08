#include <glad/glad.h>

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

static void AddVertex(std::vector<Vertex>& vertices,
                      const Vec3f&         position,
                      const Vec3f&         normal,
                      const Vec3f&         uv = Vec3f{ 0.0f, 0.0f, 0.0f })
{
    vertices.push_back(Vertex{ .position = position, .normal = normal, .uv = uv });
}

static std::vector<Vertex> CreatePlane(float halfExtent = 8.0f, float uvRepeat = 1.0f)
{
    std::vector<Vertex> vertices{};
    vertices.reserve(6);

    const Vec3f normal{ 0.0f, 1.0f, 0.0f };

    AddVertex(vertices, Vec3f{ -halfExtent, 0.0f, -halfExtent }, normal, Vec3f{ 0.0f, 0.0f, 0.0f });
    AddVertex(vertices, Vec3f{ halfExtent, 0.0f, -halfExtent }, normal, Vec3f{ uvRepeat, 0.0f, 0.0f });
    AddVertex(vertices, Vec3f{ halfExtent, 0.0f, halfExtent }, normal, Vec3f{ uvRepeat, uvRepeat, 0.0f });

    AddVertex(vertices, Vec3f{ -halfExtent, 0.0f, -halfExtent }, normal, Vec3f{ 0.0f, 0.0f, 0.0f });
    AddVertex(vertices, Vec3f{ halfExtent, 0.0f, halfExtent }, normal, Vec3f{ uvRepeat, uvRepeat, 0.0f });
    AddVertex(vertices, Vec3f{ -halfExtent, 0.0f, halfExtent }, normal, Vec3f{ 0.0f, uvRepeat, 0.0f });

    return vertices;
}

static std::vector<Vertex> CreateSphere(float radius = 0.18f, int slices = 16, int stacks = 8)
{
    std::vector<Vertex> vertices{};
    vertices.reserve(slices * stacks * 6);

    const float pi = 3.14159265359f;
    const float twoPi = 6.28318530718f;

    for ( int stack = 0; stack < stacks; ++stack )
    {
        const float phi0 = (pi * (float)stack) / (float)stacks;
        const float phi1 = (pi * (float)(stack + 1)) / (float)stacks;

        for ( int slice = 0; slice < slices; ++slice )
        {
            const float theta0 = (twoPi * (float)slice) / (float)slices;
            const float theta1 = (twoPi * (float)(slice + 1)) / (float)slices;

            const float sinPhi0 = sinf(phi0);
            const float sinPhi1 = sinf(phi1);

            const Vec3f p00{ radius * cosf(theta0) * sinPhi0, radius * sinf(theta0) * sinPhi0, radius * cosf(phi0) };
            const Vec3f p10{ radius * cosf(theta1) * sinPhi0, radius * sinf(theta1) * sinPhi0, radius * cosf(phi0) };
            const Vec3f p01{ radius * cosf(theta0) * sinPhi1, radius * sinf(theta0) * sinPhi1, radius * cosf(phi1) };
            const Vec3f p11{ radius * cosf(theta1) * sinPhi1, radius * sinf(theta1) * sinPhi1, radius * cosf(phi1) };

            if ( stack == 0 )
            {
                AddVertex(vertices, p00, Normalize(p00));
                AddVertex(vertices, p01, Normalize(p01));
                AddVertex(vertices, p11, Normalize(p11));
            }
            else if ( stack == stacks - 1 )
            {
                AddVertex(vertices, p00, Normalize(p00));
                AddVertex(vertices, p11, Normalize(p11));
                AddVertex(vertices, p10, Normalize(p10));
            }
            else
            {
                AddVertex(vertices, p00, Normalize(p00));
                AddVertex(vertices, p11, Normalize(p11));
                AddVertex(vertices, p10, Normalize(p10));

                AddVertex(vertices, p00, Normalize(p00));
                AddVertex(vertices, p01, Normalize(p01));
                AddVertex(vertices, p11, Normalize(p11));
            }
        }
    }

    return vertices;
}

struct MeshGpu
{
    GLuint vbo = 0;
    GLsizei vertexCount = 0;
    Mat4f modelMat = Mat4f::Identity();
    Vec3f baseColor{ 1.0f, 1.0f, 1.0f };
    float textureScale = 1.0f;
    bool useTexture = false;
    bool isLightMarker = false;
};

static GLuint CreateTexture2D(Image& image)
{
    GLuint handle = 0;
    glCreateTextures(GL_TEXTURE_2D, 1, &handle);
    glTextureStorage2D(handle, 1, GL_RGBA8, image.GetWidth(), image.GetHeight());
    glTextureSubImage2D(handle,
                        0,
                        0,
                        0,
                        image.GetWidth(),
                        image.GetHeight(),
                        GL_RGBA,
                        GL_UNSIGNED_BYTE,
                        image.Data());
    glTextureParameteri(handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(handle, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(handle, GL_TEXTURE_WRAP_T, GL_REPEAT);
    return handle;
}

int main(int argc, char** argv)
{
    Filesystem* pFS = Filesystem::Init(argc, argv, "assets");

    Ramen* pRamen = Ramen::Instance();
    pRamen->Init("Task 06 - Shadowmapping", 1280, 720);

    Shader sceneShader{};
    if ( !sceneShader.Load("shaders/task06.vert", "shaders/task06.frag") )
    {
        fprintf(stderr, "Could not load scene shader.\n");
    }

    Shader shadowShader{};
    if ( !shadowShader.Load("shaders/task06_shadow.vert", "shaders/task06_shadow.frag") )
    {
        fprintf(stderr, "Could not load shadow shader.\n");
    }

    Camera camera(Vec3f{ 0.0f, 2.0f, 7.0f });
    camera.RotateAroundSide(-12.0f);

    Model teapot{};
    if ( !teapot.Load("models/teapot.obj") )
    {
        fprintf(stderr, "Could not load teapot model.\n");
    }

    Image floorImage{};
    bool floorTextureLoaded = floorImage.Load("textures/linux-quake-512x512.png");
    if ( !floorTextureLoaded )
    {
        fprintf(stderr, "Could not load floor texture.\n");
    }

    GLuint floorTexture = 0;
    if ( floorTextureLoaded )
    {
        floorTexture = CreateTexture2D(floorImage);
    }
    else
    {
        glCreateTextures(GL_TEXTURE_2D, 1, &floorTexture);
        glTextureStorage2D(floorTexture, 1, GL_RGBA8, 1, 1);
        const unsigned char whitePixel[ 4 ] = { 255, 255, 255, 255 };
        glTextureSubImage2D(floorTexture, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
        glTextureParameteri(floorTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(floorTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(floorTexture, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(floorTexture, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    std::vector<Vertex> planeVertices = CreatePlane(10.0f, 8.0f);
    std::vector<Vertex> lightSphereVertices = CreateSphere(0.18f, 16, 8);

    MeshGpu planeMesh{};
    planeMesh.vertexCount = (GLsizei)planeVertices.size();
    planeMesh.baseColor = Vec3f{ 1.0f, 1.0f, 1.0f };
    planeMesh.textureScale = 1.0f;
    planeMesh.useTexture = true;

    MeshGpu teapotMesh{};
    teapotMesh.vertexCount = (GLsizei)teapot.GetVertices().size();
    teapotMesh.modelMat = Translate(Vec3f{ 0.0f, 0.35f, 0.0f }) * Scale(Vec3f{ 0.7f, 0.7f, 0.7f });
    teapotMesh.baseColor = Vec3f{ 0.78f, 0.55f, 0.32f };

    MeshGpu lightMesh{};
    lightMesh.vertexCount = (GLsizei)lightSphereVertices.size();
    lightMesh.isLightMarker = true;
    lightMesh.baseColor = Vec3f{ 1.0f, 0.95f, 0.45f };

    GLuint vao = 0;
    glCreateVertexArrays(1, &vao);
    glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glEnableVertexArrayAttrib(vao, 0);
    glVertexArrayAttribBinding(vao, 0, 0);

    glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float));
    glEnableVertexArrayAttrib(vao, 1);
    glVertexArrayAttribBinding(vao, 1, 0);

    glVertexArrayAttribFormat(vao, 2, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float));
    glEnableVertexArrayAttrib(vao, 2);
    glVertexArrayAttribBinding(vao, 2, 0);

    glVertexArrayAttribFormat(vao, 3, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float));
    glEnableVertexArrayAttrib(vao, 3);
    glVertexArrayAttribBinding(vao, 3, 0);

    glCreateBuffers(1, &planeMesh.vbo);
    glNamedBufferData(planeMesh.vbo, planeVertices.size() * sizeof(Vertex), planeVertices.data(), GL_STATIC_DRAW);

    glCreateBuffers(1, &teapotMesh.vbo);
    glNamedBufferData(teapotMesh.vbo,
                      teapot.GetVertices().size() * sizeof(Vertex),
                      teapot.GetVertices().data(),
                      GL_STATIC_DRAW);

    glCreateBuffers(1, &lightMesh.vbo);
    glNamedBufferData(lightMesh.vbo,
                      lightSphereVertices.size() * sizeof(Vertex),
                      lightSphereVertices.data(),
                      GL_STATIC_DRAW);

    const int SHADOW_MAP_SIZE = 2048;
    GLuint shadowDepthTexture = 0;
    glCreateTextures(GL_TEXTURE_2D, 1, &shadowDepthTexture);
    glTextureStorage2D(shadowDepthTexture, 1, GL_DEPTH_COMPONENT24, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
    glTextureParameteri(shadowDepthTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(shadowDepthTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(shadowDepthTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTextureParameteri(shadowDepthTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    const float borderColor[ 4 ] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTextureParameterfv(shadowDepthTexture, GL_TEXTURE_BORDER_COLOR, borderColor);

    GLuint shadowFBO = 0;
    glCreateFramebuffers(1, &shadowFBO);
    glNamedFramebufferTexture(shadowFBO, GL_DEPTH_ATTACHMENT, shadowDepthTexture, 0);
    glNamedFramebufferDrawBuffer(shadowFBO, GL_NONE);
    glNamedFramebufferReadBuffer(shadowFBO, GL_NONE);
    if ( glCheckNamedFramebufferStatus(shadowFBO, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE )
    {
        fprintf(stderr, "Shadow framebuffer is incomplete.\n");
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    bool enableShadows = true;
    bool enableGroundTexture = true;
    Vec3f lightWorldPos{ 3.0f, 5.0f, 2.0f };

    bool isRunning = true;
    SDL_GL_SetSwapInterval(1);
    Uint64 ticksPerSecond = SDL_GetPerformanceFrequency();
    Uint64 lastCounter = SDL_GetPerformanceCounter();
    float elapsedSeconds = 0.0f;

    while ( isRunning )
    {
        const Uint64 currentCounter = SDL_GetPerformanceCounter();
        const double deltaSeconds = (double)(currentCounter - lastCounter) / (double)ticksPerSecond;
        lastCounter = currentCounter;
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

            if ( e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE )
            {
                isRunning = false;
            }
        }

        const bool* keyboardState = SDL_GetKeyboardState(nullptr);
        const bool fastCamera = keyboardState[ SDL_SCANCODE_LSHIFT ] || keyboardState[ SDL_SCANCODE_RSHIFT ];
        const float moveSpeed = fastCamera ? 4.0f : 2.0f;
        const float rotateSpeed = 75.0f;
        const float moveStep = moveSpeed * (float)deltaSeconds;
        const float rotateStep = rotateSpeed * (float)deltaSeconds;

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

        int windowWidth = 0;
        int windowHeight = 0;
        SDL_GetWindowSize(pRamen->GetWindow(), &windowWidth, &windowHeight);
        glViewport(0, 0, windowWidth, windowHeight);

        Mat4f viewMat = LookAt(camera.GetPosition(), camera.GetPosition() + camera.GetForward(), camera.GetUp());
        Mat4f projMat = PerspectiveProjection(TO_RAD(60.0f), (float)windowWidth / (float)windowHeight, 0.1f, 80.0f);

        Mat4f lightViewMat = LookAt(lightWorldPos, Vec3f{ 0.0f, 0.4f, 0.0f }, RAMEN_WORLD_UP);
        Mat4f lightProjMat = PerspectiveProjection(TO_RAD(60.0f), 1.0f, 0.1f, 30.0f);
        Mat4f lightSpaceMat = lightProjMat * lightViewMat;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Task 06 - Shadowmapping");
        ImGui::Checkbox("Enable shadows", &enableShadows);
        ImGui::Checkbox("Use ground texture", &enableGroundTexture);
        ImGui::DragFloat3("Light position", lightWorldPos.Data(), 0.03f, -10.0f, 10.0f);
        ImGui::Text("Camera: %.2f %.2f %.2f", camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
        ImGui::End();

        auto drawShadowMesh = [&](const MeshGpu& mesh) {
            glVertexArrayVertexBuffer(vao, 0, mesh.vbo, 0, sizeof(Vertex));
            glUniformMatrix4fv(0, 1, GL_FALSE, mesh.modelMat.Data());
            glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount);
        };

        auto drawSceneMesh = [&](const MeshGpu& mesh) {
            glVertexArrayVertexBuffer(vao, 0, mesh.vbo, 0, sizeof(Vertex));
            glUniformMatrix4fv(0, 1, GL_FALSE, mesh.modelMat.Data());
            glUniform3fv(6, 1, mesh.baseColor.Data());
            glUniform1i(7, mesh.useTexture ? 1 : 0);
            glUniform1i(8, enableShadows ? 1 : 0);
            glUniform1f(9, mesh.textureScale);
            glUniform1i(10, mesh.isLightMarker ? 1 : 0);
            glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount);
        };

        glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
        glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        shadowShader.Use();
        glBindVertexArray(vao);
        glUniformMatrix4fv(1, 1, GL_FALSE, lightViewMat.Data());
        glUniformMatrix4fv(2, 1, GL_FALSE, lightProjMat.Data());

        drawShadowMesh(planeMesh);
        drawShadowMesh(teapotMesh);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, windowWidth, windowHeight);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.11f, 0.13f, 0.18f, 1.0f);

        sceneShader.Use();
        glBindVertexArray(vao);
        glUniformMatrix4fv(1, 1, GL_FALSE, viewMat.Data());
        glUniformMatrix4fv(2, 1, GL_FALSE, projMat.Data());
        glUniform3fv(3, 1, camera.GetPosition().Data());
        glUniform3fv(4, 1, lightWorldPos.Data());
        glUniformMatrix4fv(5, 1, GL_FALSE, lightSpaceMat.Data());
        glBindTextureUnit(0, floorTexture);
        glBindTextureUnit(1, shadowDepthTexture);

        planeMesh.useTexture = enableGroundTexture;
        drawSceneMesh(planeMesh);

        teapotMesh.useTexture = false;
        drawSceneMesh(teapotMesh);

        lightMesh.modelMat = Translate(lightWorldPos) * Scale(Vec3f{ 0.22f, 0.22f, 0.22f });
        lightMesh.baseColor = Vec3f{ 1.0f, 0.95f, 0.45f };
        lightMesh.isLightMarker = true;
        drawSceneMesh(lightMesh);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(pRamen->GetWindow());
    }

    sceneShader.Delete();
    shadowShader.Delete();
    glDeleteTextures(1, &floorTexture);
    glDeleteTextures(1, &shadowDepthTexture);
    glDeleteFramebuffers(1, &shadowFBO);
    glDeleteBuffers(1, &planeMesh.vbo);
    glDeleteBuffers(1, &teapotMesh.vbo);
    glDeleteBuffers(1, &lightMesh.vbo);
    glDeleteVertexArrays(1, &vao);

    pRamen->Shutdown();
    PHYSFS_deinit();
    return 0;
}
