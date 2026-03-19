#include <glad/glad.h>

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_video.h"
#include <stdio.h>
#include <stdlib.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

static SDL_Window*   g_pWindow;
static SDL_Renderer* g_pRenderer;
static SDL_GLContext g_glContext;

int main(int argc, char** argv)
{

    if ( !SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) )
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not initialize SDL. Reason: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    g_pWindow = SDL_CreateWindow("Task01", 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if ( !g_pWindow )
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window. Reason: %s\n", SDL_GetError());
        exit(1);
    }

    g_glContext = SDL_GL_CreateContext(g_pWindow);
    if ( !g_glContext )
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_ERROR, "Could not create OpenGL context for window. Reason: %s\n", SDL_GetError());
        exit(1);
    }

    if ( !gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress) )
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not load OpenGL extensions via GLAD.");
        exit(1);
    }

    if ( !SDL_GL_MakeCurrent(g_pWindow, g_glContext) )
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_ERROR, "Could not create OpenGL context for window. Reason: %s\n", SDL_GetError());
        exit(1);
    }

    // Print GL Version
    printf("Got OpenGL Version: %s\n", glGetString(GL_VERSION));
    printf("Got OpenGL Renderer: %s\n", glGetString(GL_RENDERER));

    bool isRunning = true;
    SDL_GL_SetSwapInterval(1); /* 1 = VSync enabledm 0 = VSync disabled */
    while ( isRunning )
    {
        SDL_Event e;
        while ( SDL_PollEvent(&e) )
        {
            if ( e.type == SDL_EVENT_QUIT )
            {
                isRunning = false;
            }
        }

        /* Rendering */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(1.0f, 0.95f, 0.0f, 1.0f);
        SDL_GL_SwapWindow(g_pWindow);
    }

    SDL_GL_DestroyContext(g_glContext);
    SDL_DestroyWindow(g_pWindow);
    SDL_Quit();

    return 0;
}
