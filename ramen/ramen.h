#ifndef RAMEN_H
#define RAMEN_H

#include "SDL3/SDL_keyboard.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glad/glad.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_impl_sdl3.h>

class Ramen
{
  public:
    static Ramen* Instance()
    {
        static Ramen* theOneAndOnly;
        if ( !theOneAndOnly )
        {
            theOneAndOnly = new Ramen();
        }
        return theOneAndOnly;
    }

    void Init(const char* title, int windowWidth, int windowHeight, const char* relBaseDir = "./")
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

        m_pWindow = SDL_CreateWindow(title, windowWidth, windowHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
        if ( !m_pWindow )
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window. Reason: %s\n", SDL_GetError());
            exit(1);
        }

        m_glContext = SDL_GL_CreateContext(m_pWindow);
        if ( !m_glContext )
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

        if ( !SDL_GL_MakeCurrent(m_pWindow, m_glContext) )
        {
            SDL_LogError(
                SDL_LOG_CATEGORY_ERROR, "Could not create OpenGL context for window. Reason: %s\n", SDL_GetError());
            exit(1);
        }

        /* Print GL version */
        printf("Got OpenGL Version: %s\n", glGetString(GL_VERSION));
        printf("Got OpenGL Renderer: %s\n", glGetString(GL_RENDERER));

        /* Init Dear ImGUI */
        /* ImGUI context */
        IMGUI_CHECKVERSION();
        m_ImGuiCtx  = ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

        /* Setup Dear ImGui style */
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsLight();

        /* ImGUI scaling */
        float       mainScale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
        ImGuiStyle& style     = ImGui::GetStyle();
        style.ScaleAllSizes(
            mainScale); // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
        style.FontScaleDpi
            = mainScale; // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true automatically overrides this for every window depending on the current monitor)

        /* Setup Platform/Renderer backends */
        ImGui_ImplSDL3_InitForOpenGL(m_pWindow, m_glContext);
        ImGui_ImplOpenGL3_Init("#version 460");

        /* Init input state */
        memset(m_Scancodes, 0, SDL_SCANCODE_COUNT * sizeof(bool));
        memset(m_PrevScancodes, 0, SDL_SCANCODE_COUNT * sizeof(bool));
    }

    void Shutdown()
    {
        /* ImGUI Shutdown */
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
        /* SDL Shutdown */
        SDL_GL_DestroyContext(m_glContext);
        SDL_DestroyWindow(m_pWindow);
        SDL_Quit();
    }

    SDL_Window* GetWindow() const
    {
        return m_pWindow;
    }

    SDL_GLContext GetGLContext() const
    {
        return m_glContext;
    }

    void EndFrame()
    {
        memcpy(m_PrevScancodes, m_Scancodes, SDL_SCANCODE_COUNT * sizeof(bool));
        // memset(m_Scancodes, 0, SDL_SCANCODE_COUNT * sizeof(bool));
    }

    void ProcessInputEvent(const SDL_Event& e)
    {
        if ( e.type == SDL_EVENT_KEY_DOWN )
        {
            m_Scancodes[ e.key.scancode ] = true;
        }
        if ( e.type == SDL_EVENT_KEY_UP )
        {
            m_Scancodes[ e.key.scancode ] = false;
        }
    }

    bool KeyWentDown(const SDL_Keycode& keycode)
    {
        SDL_Scancode sc = SDL_GetScancodeFromKey(keycode, nullptr);
        return !(m_PrevScancodes[ sc ]) && (m_Scancodes[ sc ]);
    }

    bool KeyWentUp(const SDL_Keycode& keycode)
    {
        SDL_Scancode sc = SDL_GetScancodeFromKey(keycode, nullptr);
        return (m_PrevScancodes[ sc ]) && !(m_Scancodes[ sc ]);
    }

    bool KeyPressed(const SDL_Keycode& keycode)
    {
        SDL_Scancode sc = SDL_GetScancodeFromKey(keycode, nullptr);
        return m_PrevScancodes[ sc ] && m_Scancodes[ sc ];
    }

  private:
    SDL_Window*   m_pWindow;
    SDL_GLContext m_glContext;
    SDL_Keycode   m_KeyboardState;
    SDL_Keycode   m_PrevKeyboardState;
    bool          m_Scancodes[ SDL_SCANCODE_COUNT ];
    bool          m_PrevScancodes[ SDL_SCANCODE_COUNT ];
    ImGuiContext* m_ImGuiCtx;
    Ramen() = default;
};

#endif
