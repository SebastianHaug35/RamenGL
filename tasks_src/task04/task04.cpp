#include <glad/glad.h>

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <vector>

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

#define NUM_QUAD_VERTICES 4
#define NUM_QUAD_INDICES 6

static Vertex quadVertices[ NUM_QUAD_VERTICES ] = {
    {
        .position = Vec3f{ -1.0f, -1.0f, 0.0f },
        .uv       = Vec3f{ 0.0f, 1.0f, 0.0f },
    },
    {
        .position = Vec3f{ 1.0f, -1.0f, 0.0f },
        .uv       = Vec3f{ 1.0f, 1.0f, 0.0f },
    },
    {
        .position = Vec3f{ 1.0f, 1.0f, 0.0f },
        .uv       = Vec3f{ 1.0f, 0.0f, 0.0f },
    },
    {
        .position = Vec3f{ -1.0f, 1.0f, 0.0f },
        .uv       = Vec3f{ 0.0f, 0.0f, 0.0f },
    },
};
static uint16_t quadIndices[ NUM_QUAD_INDICES ]   = { 0, 1, 2, 2, 3, 0 };

static Vec3f SpherePoint(float radius, float theta, float phi)
{
    const float sinPhi = sinf(phi);

    return Vec3f{
        radius * cosf(theta) * sinPhi,
        radius * sinf(theta) * sinPhi,
        radius * cosf(phi),
    };
}

static Vertex MakeSphereVertex(float radius, float theta, float phi)
{
    Vec3f position = SpherePoint(radius, theta, phi);

    float u = theta / 6.28318530718f;
    float v = phi / 3.14159265359f;

    return Vertex{
        .position = position,
        .normal   = Normalize(position),
        .color    = Vec3f{ 1.0f, 1.0f, 1.0f },
        .uv       = Vec3f{ u, v, 0.0f },
    };
}

static std::vector<Vertex> CreateTexturedSphere(int slices = 64, int stacks = 32)
{
    std::vector<Vertex> vertices{};

    const float radius = 1.0f;
    const float pi     = 3.14159265359f;
    const float twoPi  = 6.28318530718f;

    vertices.reserve(slices * stacks * 6);

    for ( int stack = 0; stack < stacks; stack++ )
    {
        float phi0 = pi * (float)stack / (float)stacks;
        float phi1 = pi * (float)(stack + 1) / (float)stacks;

        for ( int slice = 0; slice < slices; slice++ )
        {
            float theta0 = twoPi * (float)slice / (float)slices;
            float theta1 = twoPi * (float)(slice + 1) / (float)slices;

            Vertex p00 = MakeSphereVertex(radius, theta0, phi0);
            Vertex p10 = MakeSphereVertex(radius, theta1, phi0);
            Vertex p01 = MakeSphereVertex(radius, theta0, phi1);
            Vertex p11 = MakeSphereVertex(radius, theta1, phi1);

            if ( stack == 0 )
            {
                vertices.push_back(p00);
                vertices.push_back(p11);
                vertices.push_back(p01);
            }
            else if ( stack == stacks - 1 )
            {
                vertices.push_back(p00);
                vertices.push_back(p10);
                vertices.push_back(p11);
            }
            else
            {
                vertices.push_back(p00);
                vertices.push_back(p10);
                vertices.push_back(p11);

                vertices.push_back(p00);
                vertices.push_back(p11);
                vertices.push_back(p01);
            }
        }
    }

    return vertices;
}


int main(int argc, char** argv)
{
    Filesystem* pFS = Filesystem::Init(argc, argv);

    Ramen* pRamen = Ramen::Instance();
    pRamen->Init("Model loading", 800, 600);

    /* Load shaders. */
    Shader shader{};
    if ( !shader.Load("shaders/textures.vert", "shaders/textures.frag") )
    {
        fprintf(stderr, "Could not load shader.\n");
    }

    Image image{};
    if ( !image.Load("textures/world.200404.3x5400x2700.jpg") )
    {
        fprintf(stderr, "Could not load texture file.\n");
    }

    GLuint textureHandle = 0;
    glCreateTextures(GL_TEXTURE_2D, 1, &textureHandle);

    glTextureParameteri(textureHandle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(textureHandle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTextureParameteri(textureHandle, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(textureHandle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTextureStorage2D(
        textureHandle,
        1,
        GL_RGBA8,
        image.GetWidth(),
        image.GetHeight());

    glTextureSubImage2D(
        textureHandle,
        0,
        0,
        0,
        image.GetWidth(),
        image.GetHeight(),
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        image.Data());

    std::vector<Vertex> sphereVertices = CreateTexturedSphere();


    /* Create GPU buffer and upload model's vertices. */
    GLuint VBO;
    glCreateBuffers(1, &VBO);
    glNamedBufferData(VBO, sphereVertices.size() * sizeof(Vertex), sphereVertices.data(), GL_STATIC_DRAW);
    GLuint EBO;
    glCreateBuffers(1, &EBO);
    glNamedBufferData(EBO, NUM_QUAD_INDICES * sizeof(uint16_t), quadIndices, GL_STATIC_DRAW);

    /* Create camera */
    Camera camera(Vec3f{ 0.0f, 0.0f, 5.0f });
    camera.RotateAroundSide(0.0f);

    /* Model mat*/
    Mat4f modelMat = Mat4f::Identity();

    /* VAO. */
    GLuint VAO;
    glCreateVertexArrays(1, &VAO);
    glVertexArrayVertexBuffer(VAO, 0, VBO, 0, sizeof(Vertex));
    glVertexArrayElementBuffer(VAO, EBO);
    /* Position */
    glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glEnableVertexArrayAttrib(VAO, 0);
    glVertexArrayAttribBinding(VAO, 0, 0);
    /* Normal */
    glVertexArrayAttribFormat(VAO, 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float));
    glEnableVertexArrayAttrib(VAO, 1);
    glVertexArrayAttribBinding(VAO, 1, 0);
    /* Color */
    glVertexArrayAttribFormat(VAO, 2, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float));
    glEnableVertexArrayAttrib(VAO, 2);
    glVertexArrayAttribBinding(VAO, 2, 0);
    /* UV */
    glVertexArrayAttribFormat(VAO, 3, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float));
    glEnableVertexArrayAttrib(VAO, 3);
    glVertexArrayAttribBinding(VAO, 3, 0);

    /* Some global GL states */
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    // glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

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
        startCounter         = SDL_GetPerformanceCounter();

        float deltaSeconds = (float)(msPerFrame / 1000.0);

        SDL_Event e;
        while ( SDL_PollEvent(&e) )
        {
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

        /* Query new frame dimensions */
        int windowWidth, windowHeight;
        SDL_GetWindowSize(pRamen->GetWindow(), &windowWidth, &windowHeight);

        /* Adjust viewport and perspective projection accordingly. */
        glViewport(0, 0, windowWidth, windowHeight);

        /* View mat */
        Mat4f viewMat = LookAt(
            camera.GetPosition(), camera.GetPosition() + camera.GetForward(), camera.GetUp()); // Mat4f::Identity();

        /* Projection mat */
        float aspect  = (float)windowWidth / (float)windowHeight;
        Mat4f projMat = PerspectiveProjection(TO_RAD(60.0f), aspect, 0.01f, 500.0f);

        /* Rendering */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);

        shader.Use();
        glBindVertexArray(VAO);
        glBindTextureUnit(0, textureHandle);

        glUniformMatrix4fv(0, 1, GL_FALSE, modelMat.Data());
        glUniformMatrix4fv(1, 1, GL_FALSE, viewMat.Data());
        glUniformMatrix4fv(2, 1, GL_FALSE, projMat.Data());

        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)sphereVertices.size());

        SDL_GL_SwapWindow(pRamen->GetWindow());

        endCounter = SDL_GetPerformanceCounter();
    }

    /* GL Resources shutdown. */
    shader.Delete();
    glDeleteTextures(1, &textureHandle);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);



    /* Ramen Shutdown */
    pRamen->Shutdown();

    /* Filesystem deinit */
    PHYSFS_deinit();

    return 0;
}
