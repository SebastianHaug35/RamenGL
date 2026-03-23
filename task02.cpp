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

#include "ramen/ramen.h"
#include "ramen/rgl_camera.h"
#include "ramen/rgl_defines.h"
#include "ramen/rgl_math.h"
#include "ramen/rgl_model.h"
#include "ramen/rgl_shader.h"

int main(int argc, char** argv)
{
    Ramen* pRamen = Ramen::Create();
    pRamen->Init("Task 02", 800, 600);

    /* Load shaders. */
    Shader shader{};
    if ( !shader.Load("./shaders/task01.vert", "./shaders/task01.frag") )
    {
        fprintf(stderr, "Could not load shader.\n");
    }

    /* Load model data from disk */
    Model model{};
    if ( !model.Load("./models/stormtrooper.obj") )
    {
        fprintf(stderr, "Could not load model file.\n");
    }

    // TODO: Create vertex layout via VAO.
    // TODO: Create a buffer on GPU and upload the model's vertices.

    /* Create camera */
    Camera camera(Vec3f{ 0.0f, 0.0f, 20.0f }, Vec3f{ 0.0f, 0.0f, 0.0f });

    /* Model mat*/
    Mat4f modelMat = Mat4f::Identity();

    /*  Gobal GL states */
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    SDL_GL_SetSwapInterval(1); /* 1 = VSync enabled; 0 = VSync disabled */

    /* Main loop */
    bool isRunning = true;
    while ( isRunning )
    {
        SDL_Event e;
        while ( SDL_PollEvent(&e) )
        {
            ImGui_ImplSDL3_ProcessEvent(&e);

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
        Mat4f projMat = PerspectiveProjection(TO_RAD(70.0f), aspect, 0.01f, 500.0f);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // NOTE: Bonus: You can uncomment this and check out
        // what the UI library can do. We will work with it later.
        // ImGui::ShowDemoWindow();

        /* ImGUI Rendering */
        ImGui::Render();

        /* Rendering */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(1.0f, 0.95f, 0.0f, 1.0f);

        shader.Use();

        // TODO: Activate VAO.

        glUniformMatrix4fv(0, 1, GL_FALSE, modelMat.Data());
        glUniformMatrix4fv(1, 1, GL_FALSE, viewMat.Data());
        glUniformMatrix4fv(2, 1, GL_FALSE, projMat.Data());

        // TODO: Draw the model.

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(pRamen->GetWindow());
    }

    /* GL Resources shutdown. */
    shader.Delete();

    // TODO: Delete OpenGL Resources you created (VAO, VBO).

    /* Ramen Shutdown */
    pRamen->Shutdown();

    return 0;
}
