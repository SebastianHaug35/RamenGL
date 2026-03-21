#include <glad/glad.h>

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_video.h"
#include <assert.h>
#include <math.h>
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
    Vec3f(float _x, float _y, float _z)
    {
        x = _x;
        y = _y;
        z = _z;
    }

    Vec3f(float s)
    {
        x = s;
        y = s;
        z = s;
    }

    Vec3f()
    {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
    }

    union
    {
        float x, y, z;
        float components[ 3 ];
    };

    const float operator[](const int index) const
    {
        assert(index < 3);
        return components[ index ];
    }

    float& operator[](const int index)
    {
        assert(index < 3);
        return components[ index ];
    }

    Vec3f operator-(const Vec3f& right) const
    {
        return Vec3f{ x - right.x, y - right.y, z - right.z };
    }

    Vec3f operator+(const Vec3f& right) const
    {
        return Vec3f{ x + right.x, y + right.y, z + right.z };
    }

    Vec3f operator/(float s) const
    {
        return Vec3f{ x / s, y / s, z / s };
    }
};

struct Vec4f
{
    Vec4f(float _x, float _y, float _z, float _w)
    {
        x = _x;
        y = _y;
        z = _z;
        w = _w;
    }

    Vec4f(float s)
    {
        x = s;
        y = s;
        z = s;
        w = s;
    }

    Vec4f()
    {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
        w = 0.0f;
    }

    Vec4f(const Vec3f& v3)
    {
        Vec4f{ v3.x, v3.y, v3.z, 1.0f };
    }

    union
    {
        float x, y, z, w;
        float components[ 4 ];
    };

    const float operator[](const int index) const
    {
        assert(index < 4);
        return components[ index ];
    }

    Vec4f operator-(const Vec4f& right) const
    {
        return Vec4f{ x - right.x, y - right.y, z - right.z, w - right.w };
    }

    Vec4f operator+(const Vec4f& right) const
    {
        return Vec4f{ x + right.x, y + right.y, z + right.z, w - right.w };
    }

    float& operator[](const int index)
    {
        assert(index < 4);
        return components[ index ];
    }

    Vec4f operator/(float s) const
    {
        return Vec4f{ x / s, y / s, z / s, w / s };
    }
};

struct Mat4f
{
    union
    {
        float elements[ 16 ];
        Vec4f columns[ 4 ];
    };

    Mat4f()
    {
        columns[ 0 ] = Vec4f{};
        columns[ 1 ] = Vec4f{};
        columns[ 2 ] = Vec4f{};
        columns[ 3 ] = Vec4f{};
    }

    Mat4f(const Vec4f& col1, const Vec4f& col2, const Vec4f& col3, const Vec4f& col4)
    {
        columns[ 0 ] = col1;
        columns[ 1 ] = col2;
        columns[ 2 ] = col3;
        columns[ 3 ] = col4;
    }

    static Mat4f Identity()
    {
        return Mat4f{ Vec4f{ 1.0f, 0.0f, 0.0f, 0.0f },
                      Vec4f{ 0.0f, 1.0f, 0.0f, 0.0f },
                      Vec4f{ 0.0f, 0.0f, 1.0f, 0.0f },
                      Vec4f{ 0.0f, 0.0f, 0.0f, 1.0f } };
    }
};

Vec3f Cross(const Vec3f& a, const Vec3f& b)
{
    return Vec3f{ a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}

float Length(const Vec3f& v)
{
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

float Length(const Vec4f& v)
{
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}

Vec3f Normalize(const Vec3f& v)
{
    float len = Length(v);
    return v / len;
}

Vec4f Normalize(const Vec4f& v)
{
    float len = Length(v);
    return v / len;
}

struct Vertex
{
    Vec3f position;
    Vec3f normal;
    Vec3f color;
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
                tinyobj::index_t idx = m_Shapes[ i ].mesh.indices[ j ];
                Vertex           v{};
                for ( int k = 0; k < 3; k++ )
                {
                    v.position[ k ] = attrib.vertices[ 3 * idx.vertex_index + k ];
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

class Camera
{
  public:
    Camera(const Vec3f& position, const Vec3f& target)
    {
        Vec3f forward = Normalize(target - position);
        m_Position    = position;
        m_Forward     = forward;
        Vec3f up      = Vec3f{ 0.0f, 1.0f, 0.0f };
        Vec3f right   = Normalize(Cross(m_Forward, up));
        m_Up          = Normalize(Cross(right, m_Forward));
    }

    const Vec3f& GetPosition() const
    {
        return m_Position;
    }

    const Vec3f& GetForward() const
    {
        return m_Forward;
    }

    const Vec3f& GetUp() const
    {
        return m_Up;
    }

    const Vec3f GetRight() const
    {
        return Normalize(Cross(m_Forward, m_Up));
    }

  private:
    Vec3f m_Position;
    Vec3f m_Forward;
    Vec3f m_Up;
};

Mat4f LookAt(const Camera& camera)
{
    return Mat4f{ camera.GetRight(), camera.GetUp(), camera.GetForward(), camera.GetPosition() };
}

static std::vector<Vertex> g_Cone;
static std::vector<Vertex> g_Cap;
static GLuint              g_ConeVBO;
static GLuint              g_CapVBO;

#define RAMEN_PI 3.14f
void CreateGeometry()
{
    // Triangle Fan mit 18 Vertices anlegen
    // Die Spitze des Konus ist ein Vertex, den alle Triangles gemeinsam haben;
    // um einen Konus anstatt einen Kreis zu produzieren muss der Vertex einen positiven z-Wert haben
    g_Cone.push_back(Vertex{ .position = Vec3f{ 0.0f, 0.0f, 75.0f }, .normal = Vec3f{}, .color = Vec3f{ 0, 1, 0 } });
    // Kreise um den Mittelpunkt und spezifiziere Vertices entlang des Kreises
    // um einen Triangle_Fan zu erzeugen
    int iPivot = 1;
    for ( float angle = 0.0f; angle < (2.0f * RAMEN_PI); angle += (RAMEN_PI / 8.0f) )
    {
        // Berechne x und y Positionen des naechsten Vertex
        float x = 50.0f * sin(angle);
        float y = 50.0f * cos(angle);

        Vertex v{};

        // Alterniere die Farbe
        if ( (iPivot % 2) == 0 )
            v.color = Vec3f{ 0.235f, 0.235f, 0.235f };
        else
            v.color = Vec3f{ 0.0f, 0.6f, 1.0f };

        // Inkrementiere iPivot um die Farbe beim naechsten mal zu wechseln
        iPivot++;

        // Spezifiziere den naechsten Vertex des Triangle_Fans
        v.position = Vec3f{ x, y, 0.0f };
        g_Cone.push_back(v);
    }

    // Fertig mit dem Konus
    /*
    // Erzeuge einen weiteren Triangle_Fan um den Boden zu bedecken
    boden.Begin(GL_TRIANGLE_FAN, 18);
    // Das Zentrum des Triangle_Fans ist im Ursprung
    boden.Vertex3f(0.0f, 0.0f, 0.0f);
    for ( float angle = 0.0f; angle < (2.0f * GL_PI); angle += (GL_PI / 8.0f) )
    {
        // Berechne x und y Positionen des naechsten Vertex
        float x = 50.0f * sin(angle);
        float y = 50.0f * cos(angle);

        // Alterniere die Farbe
        if ( (iPivot % 2) == 0 )
            boden.Color4f(1, 0.8, 0.2, 1);
        else
            boden.Color4f(0, 0.8, 0, 1);

        // Inkrementiere iPivot um die Farbe beim naechsten mal zu wechseln
        iPivot++;

        // Spezifiziere den naechsten Vertex des Triangle_Fans
        boden.Vertex3f(x, y, 0);
    }

    // Fertig mit dem Bodens
    boden.End();
    */

    glCreateBuffers(1, &g_ConeVBO);
    glNamedBufferData(g_ConeVBO, g_Cone.size() * sizeof(Vertex), g_Cone.data(), GL_STATIC_DRAW);
}

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

    /* Create a model on GPU */
    Model model{};
    if ( !model.Load("./models/Skull.obj") )
    {
        fprintf(stderr, "Could not load model file.\n");
    }

    /* Create Cylinder and upload to GPU */
    CreateGeometry();

    /* Create camera */
    Camera camera(Vec3f{ 0.0f, 0.0f, -10.0f }, Vec3f{ 0.0f });

    /* VAO. */
    GLuint VAO;
    glCreateVertexArrays(1, &VAO);
    glVertexArrayVertexBuffer(VAO, 0, model.GetBuffer(), 0, sizeof(Vertex));
    glVertexArrayAttribBinding(VAO, 0, 0);
    glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glEnableVertexArrayAttrib(VAO, 0);

    /* Main loop */
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
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, model.NumVertices());

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
