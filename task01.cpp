#include <glad/glad.h>

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_video.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <vector>

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

static SDL_Window*   g_pWindow;
static SDL_Renderer* g_pRenderer;
static SDL_GLContext g_glContext;

bool FileExists(const char* file)
{
    FILE* pFileDesc = nullptr;
    pFileDesc       = fopen(file, "r");
    if ( pFileDesc == nullptr )
    {
        return false;
    }
    fclose(pFileDesc);

    return true;
}

struct File
{
    void Destroy()
    {
        if ( data )
        {
            free(data);
            size = 0;
        }
    }

    char*  data;
    size_t size;
};

File ReadFile(const char* file)
{
    FILE* pFileDesc = 0;
    pFileDesc       = fopen(file, "rb");
    if ( file == 0 )
    {
        return File{};
    }
    fseek(pFileDesc, 0L, SEEK_END);
    size_t size = ftell(pFileDesc);
    fseek(pFileDesc, 0L, SEEK_SET);
    char* data = (char*)malloc(size + 1);
    fread(data, sizeof(char), size, pFileDesc);
    data[ size ] = '\0';
    fclose(pFileDesc);

    return File{ data, size };
}

struct Vec3f
{
    float x, y, z;
};

struct Vertex
{
    union
    {
        Vec3f position;
        float positions[ 3 ];
    };

    const float operator[](const int index) const
    {
        assert(index < 3);
        return positions[ index ];
    }

    float& operator[](const int index)
    {
        return positions[ index ];
    }
};

class Shader
{
  public:
    bool Load(const char* vertShaderFile, const char* fragShaderFile)
    {
        GLuint vertShader;
        GLuint fragShader;

        if ( !CompileShader(vertShaderFile, &vertShader, GL_VERTEX_SHADER) )
        {
            fprintf(stderr, "Failed to compile shader: '%s'\n", vertShaderFile);
            return false;
        }
        if ( !CompileShader(fragShaderFile, &fragShader, GL_FRAGMENT_SHADER) )
        {
            fprintf(stderr, "Failed to compile shader: '%s'\n", fragShaderFile);
            return false;
        }

        m_Program = glCreateProgram();
        glAttachShader(m_Program, vertShader);
        glAttachShader(m_Program, fragShader);
        glLinkProgram(m_Program);

        glDeleteShader(vertShader);
        glDeleteShader(fragShader);

        return true;
    }

    void Use()
    {
        glUseProgram(m_Program);
    }

    void Delete()
    {
        glDeleteProgram(m_Program);
    }

  private:
    bool CompileShader(const char* shaderFilename, GLuint* shader, GLenum shaderType)
    {
        if ( !FileExists(shaderFilename) )
        {
            fprintf(stderr, "Shader file: '%s' does not exist.\n", shaderFilename);
            return false;
        }

        /* Read shader source from disk and compile. */
        File shaderFile = ReadFile(shaderFilename);
        *shader         = glCreateShader(shaderType);
        glShaderSource(*shader, 1, &shaderFile.data, nullptr);
        glCompileShader(*shader);
        if ( !IsCompiled(*shader) )
        {
            return false;
        }

        return true;
    }

    bool IsCompiled(GLuint shader)
    {
        GLint status;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if ( status != GL_TRUE )
        {
            char buffer[ 512 ];
            memset(buffer, 0, 512);
            glGetShaderInfoLog(shader, 511, nullptr, buffer);
            printf("GLSL compile error:\n%s\n", buffer);

            return false;
        }

        return true;
    }

  private:
    GLuint m_Program;
};

class Model
{
  public:
    bool Load(const char* file)
    {
        if ( !FileExists(file) )
        {
            return false;
        }

        File              modelFile = ReadFile(file);
        tinyobj::attrib_t attrib;
        std::string       err;
        bool              ret = tinyobj::LoadObj(&attrib, &m_Shapes, nullptr, &err, file, "/");
        if ( !ret )
        {
            return false;
        }

        printf("loaded: '%s'\n", file);
        printf("# of vertices: %d\n", (int)(attrib.vertices.size()) / 3);
        printf("# of normals: %d\n", (int)(attrib.normals.size() / 3));
        printf("# of texcoords: %d\n", (int)attrib.texcoords.size() / 2);
        printf("# of shapes: %d\n", (int)m_Shapes.size());

        for ( int i = 0; i < m_Shapes.size(); i++ )
        {
            for ( int j = 0; j < m_Shapes[ i ].mesh.indices.size(); j++ )
            {
                Vertex v{};
                for ( int k = 0; k < 3; k++ )
                {
                    tinyobj::index_t idx = m_Shapes[ i ].mesh.indices[ j ];

                    float component = attrib.vertices[ idx.vertex_index ];
                    v[ k ]          = component;
                }
                m_Vertices.push_back(v);
            }
        }
        printf("m_Vertices.size(): %lu\n", m_Vertices.size());

        UploadToGPU();

        modelFile.Destroy();

        return true;
    }

    const GLuint GetBuffer()
    {
        return m_Buffer;
    }

    const size_t NumVertices()
    {
        return m_Vertices.size();
    }

  private:
    void UploadToGPU()
    {
        glCreateBuffers(1, &m_Buffer);
        glNamedBufferData(m_Buffer, m_Vertices.size() * sizeof(Vertex), m_Vertices.data(), GL_STATIC_DRAW);
    }

    void UnloadFromGPU()
    {
        if ( m_Buffer != 0 )
        {
            glDeleteBuffers(1, &m_Buffer);
        }
    }

  private:
    std::vector<tinyobj::shape_t> m_Shapes;
    std::vector<Vertex>           m_Vertices;
    GLuint                        m_Buffer;
};

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

    /* Print GL version */
    printf("Got OpenGL Version: %s\n", glGetString(GL_VERSION));
    printf("Got OpenGL Renderer: %s\n", glGetString(GL_RENDERER));

    /* Load shaders. */
    Shader shader{};
    if ( !shader.Load("./shaders/task01.vert", "./shaders/task01.frag") )
    {
        fprintf(stderr, "Could not load shader.\n");
    }

    /* Dummy VAO. */
    GLuint VAO;
    glCreateVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    /* Create a model on GPU */
    Model model{};
    if ( !model.Load("./models/cylinder.obj") )
    {
        fprintf(stderr, "Could not load model file.\n");
    }

    bool isRunning = true;
    SDL_GL_SetSwapInterval(1); /* 1 = VSync enabledm 0 = VSync disabled */
    glPointSize(20.0f);        // TODO: Delete later.
    while ( isRunning )
    {
        SDL_Event e;
        while ( SDL_PollEvent(&e) )
        {
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

        /* Rendering */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(1.0f, 0.95f, 0.0f, 1.0f);

        shader.Use();
        // glBindBuffer(GL_ARRAY_BUFFER, model.GetBuffer());
        glDrawArrays(GL_POINTS, 0, 1);

        SDL_GL_SwapWindow(g_pWindow);
    }

    /* GL Resources shutdown. */
    shader.Delete();
    glDeleteVertexArrays(1, &VAO);

    /* SDL Shutdown */
    SDL_GL_DestroyContext(g_glContext);
    SDL_DestroyWindow(g_pWindow);
    SDL_Quit();

    return 0;
}
