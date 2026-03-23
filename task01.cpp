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

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "ramen/rgl_defines.h"
#include "ramen/rgl_math.h"
#include "ramen/rgl_platform.h"

static SDL_Window*   g_pWindow;
static SDL_Renderer* g_pRenderer;
static SDL_GLContext g_glContext;

/* 4x4 Matrix with floats.
 * Stores elements in column major order.
 * Constructors also expect data in columns!
 */
struct Mat4f
{
    float e[ 4 ][ 4 ];

    Mat4f() = default;

    Mat4f(
        /* 1st col */
        const float& d00,
        const float& d01,
        const float& d02,
        const float& d03,
        /* 2nd col */
        const float& d10,
        const float& d11,
        const float& d12,
        const float& d13,
        /* 3rd col */
        const float& d20,
        const float& d21,
        const float& d22,
        const float& d23,
        /* 4th col */
        const float& d30,
        const float& d31,
        const float& d32,
        const float& d33)
    {
        e[ 0 ][ 0 ] = d00;
        e[ 0 ][ 1 ] = d01;
        e[ 0 ][ 2 ] = d02;
        e[ 0 ][ 3 ] = d03;

        e[ 1 ][ 0 ] = d10;
        e[ 1 ][ 1 ] = d11;
        e[ 1 ][ 2 ] = d12;
        e[ 1 ][ 3 ] = d13;

        e[ 2 ][ 0 ] = d20;
        e[ 2 ][ 1 ] = d21;
        e[ 2 ][ 2 ] = d22;
        e[ 2 ][ 3 ] = d23;

        e[ 3 ][ 0 ] = d30;
        e[ 3 ][ 1 ] = d31;
        e[ 3 ][ 2 ] = d32;
        e[ 3 ][ 3 ] = d33;
    }

    Mat4f(const Vec4f& col1, const Vec4f& col2, const Vec4f& col3, const Vec4f& col4)
    {
        e[ 0 ][ 0 ] = col1.x;
        e[ 0 ][ 1 ] = col1.y;
        e[ 0 ][ 2 ] = col1.z;
        e[ 0 ][ 3 ] = col1.w;
        e[ 1 ][ 0 ] = col2.x;
        e[ 1 ][ 1 ] = col2.y;
        e[ 1 ][ 2 ] = col2.z;
        e[ 1 ][ 3 ] = col2.w;
        e[ 2 ][ 0 ] = col3.x;
        e[ 2 ][ 1 ] = col3.y;
        e[ 2 ][ 2 ] = col3.z;
        e[ 2 ][ 3 ] = col3.w;
        e[ 3 ][ 0 ] = col4.x;
        e[ 3 ][ 1 ] = col4.y;
        e[ 3 ][ 2 ] = col4.z;
        e[ 3 ][ 3 ] = col4.w;
    }

    /* Follows mathematical notation:
     * i = row, j = column.
     */
    const float& operator()(int i, int j) const
    {
        return e[ j ][ i ];
    }

    float& operator()(int i, int j)
    {
        return e[ j ][ i ];
    }

    const Vec4f& operator[](int col) const
    {
        return reinterpret_cast<const Vec4f&>(e[ col ]);
    }

    Vec4f& operator[](int col)
    {
        return reinterpret_cast<Vec4f&>(e[ col ]);
    }

    void Transpose()
    {
        Mat4f result{};
        for ( int i = 0; i < 4; i++ )
        {
            const Vec4f& col = this->operator[](i);
            for ( int j = 0; j < 4; j++ )
            {
                result[ j ][ i ] = col[ j ];
            }
        }
        *this = result;
    }

    Mat4f operator*(const Mat4f& rhs)
    {
        Mat4f result{};
        for ( int row = 0; row < 4; row++ )
        {
            for ( int col = 0; col < 4; col++ )
            {
                float sum = 0.0f;
                for ( int k = 0; k < 4; k++ )
                {
                    sum += (*this)(row, k) * rhs(k, col);
                }

                result(row, col) = sum;
            }
        }

        return result;
    }

    Vec4f operator*(const Vec4f& v)
    {
        Vec4f result{};
        for ( int row = 0; row < 4; row++ )
        {
            float value = 0.0f;
            for ( int k = 0; k < 4; k++ )
            {
                value += (*this)(row, k) * v[ k ];
            }
            result[ row ] = value;
        }

        return result;
    }

    static Mat4f Identity()
    {
        return Mat4f{ Vec4f{ 1.0f, 0.0f, 0.0f, 0.0f },
                      Vec4f{ 0.0f, 1.0f, 0.0f, 0.0f },
                      Vec4f{ 0.0f, 0.0f, 1.0f, 0.0f },
                      Vec4f{ 0.0f, 0.0f, 0.0f, 1.0f } };
    }

    const float* Data()
    {
        return (float*)(e[ 0 ]);
    }

    const char* ToString() const
    {
        static char buffer[ 256 ];
        sprintf(buffer,
                "%.2f, %.2f, %.2f, %.2f\n%.2f, %.2f, %.2f, %.2f\n%.2f, %.2f, %.2f, %.2f\n%.2f, %.2f, %.2f, %.2f\n",
                e[ 0 ][ 0 ],
                e[ 1 ][ 0 ],
                e[ 2 ][ 0 ],
                e[ 3 ][ 0 ],
                e[ 0 ][ 1 ],
                e[ 1 ][ 1 ],
                e[ 2 ][ 1 ],
                e[ 3 ][ 1 ],
                e[ 0 ][ 2 ],
                e[ 1 ][ 2 ],
                e[ 2 ][ 2 ],
                e[ 3 ][ 2 ],
                e[ 0 ][ 3 ],
                e[ 1 ][ 3 ],
                e[ 2 ][ 3 ],
                e[ 3 ][ 3 ]);
        return buffer;
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

float Dot(const Vec3f& a, const Vec3f& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec4f Normalize(const Vec4f& v)
{
    float len = Length(v);
    return v / len;
}

float Dot(const Vec4f& a, const Vec4f& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

/* Create Inverse of 4x4 Matrix.
 * Code from 'Foundations of Game Engine programming Vol.1'
 * by Eric Lengyel.
 */
Mat4f Inverse(const Mat4f& m)
{
    const Vec3f& a = reinterpret_cast<const Vec3f&>(m[ 0 ]);
    const Vec3f& b = reinterpret_cast<const Vec3f&>(m[ 1 ]);
    const Vec3f& c = reinterpret_cast<const Vec3f&>(m[ 2 ]);
    const Vec3f& d = reinterpret_cast<const Vec3f&>(m[ 3 ]);

    const float& x = m(3, 0);
    const float& y = m(3, 1);
    const float& z = m(3, 2);
    const float& w = m(3, 3);

    Vec3f s = Cross(a, b);
    Vec3f t = Cross(c, d);
    Vec3f u = a * y - b * x;
    Vec3f v = c * w - d * z;

    float invDet = 1.0f / (Dot(s, v) + Dot(t, u));
    s *= invDet;
    t *= invDet;
    u *= invDet;
    v *= invDet;

    Vec3f r0 = Cross(b, v) + t * y;
    Vec3f r1 = Cross(v, a) - t * x;
    Vec3f r2 = Cross(d, u) + s * w;
    Vec3f r3 = Cross(u, c) - s * z;

    return Mat4f(r0.x,
                 r0.y,
                 r0.z,
                 -Dot(b, t),
                 r1.x,
                 r1.y,
                 r1.z,
                 Dot(a, t),
                 r2.x,
                 r2.y,
                 r2.z,
                 -Dot(d, s),
                 r3.x,
                 r3.y,
                 r3.z,
                 Dot(c, s));
}

struct Quat
{
    float x, y, z, w;

    const Quat operator*(const Quat& rhs) const
    {
        return Quat{ rhs.w * w - rhs.x * x - rhs.y * y - rhs.z * z,
                     rhs.w * x + rhs.x * w - rhs.y * z + rhs.z * y,
                     rhs.w * y + rhs.x * z + rhs.y * w - rhs.z * x,
                     rhs.w * z - rhs.x * y + rhs.y * x + rhs.z * w };
    }

    Quat& operator/=(const float s)
    {
        x /= s;
        y /= s;
        z /= s;
        w /= s;
        return *this;
    }
};

/* Builds a versor from given axis and rotation angle, given in degrees.
 * A versor is the expression of a rotation in a '4D rotational vector'.
 * Note that this computes a unit-quaternion.
 * Also note that mathematicians don't like it when we call Quaternions Vectors!
 */
Quat AngleAxis(const float& x, const float& y, const float& z, const float& angleDgr)
{
    Quat result{};

    const float& angleRad     = TO_RAD(angleDgr);
    const float& halfTheta    = angleRad * 0.5f;
    const float& sinHalfTheta = sinf(halfTheta);
    result.x                  = sinHalfTheta * x;
    result.y                  = sinHalfTheta * y;
    result.z                  = sinHalfTheta * z;
    result.w                  = cosf(halfTheta);

    return result;
}

Quat AngleAxis(const Vec3f& v, const float& angleDgr)
{
    return AngleAxis(v.x, v.y, v.z, angleDgr);
}

Mat4f AsMat4f(Quat& q)
{
    float len2 = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
    if ( fabs(len2 - 1.0f) > RAMEN_QUAT_EPSILON )
    {
        q /= sqrt(len2);
    }

    const float& x  = q.x;
    const float& y  = q.y;
    const float& z  = q.z;
    const float& w  = q.w;
    const float  x2 = x * x;
    const float  y2 = y * y;
    const float  z2 = z * z;
    const float  w2 = w * w;

    return Mat4f{ Vec4f{ 1.0f - 2.0f * y2 - 2.0f * z2, 2.0f * x * y + 2.0f * w * z, 2.0f * x * z - 2.0f * w * y, 0.0f },
                  Vec4f{ 2.0f * x * y - 2.0f * w * z, 1.0f - 2.0f * x2 - 2.0f * z2, 2.0f * y * z + 2.0f * w * x, 0.0f },
                  Vec4f{ 2.0f * x * z + 2.0f * w * y, 2.0f * y * z - 2.0f * w * x, 1.0f - 2.0f * x2 - 2.0f * y2, 0.0f },
                  Vec4f{ 0.0f, 0.0f, 0.0f, 1.0f } };
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
        Vec3f forward  = Normalize(target - position);
        m_Position     = position;
        m_Forward      = forward;
        Vec3f up       = Vec3f{ 0.0f, 1.0f, 0.0f };
        Vec3f right    = Normalize(Cross(m_Forward, up));
        m_Up           = Normalize(Cross(right, m_Forward));
        m_qOrientation = AngleAxis(m_Forward, 0.0f);
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

    /* Orient camera so that it looks at target. */
    void Orient(const Vec3f target)
    {
        Vec3f newForward = Normalize(target - m_Position);
        Vec3f newSide    = Normalize(Cross(newForward, Vec3f{ 0.0f, 1.0f, 0.0f }));
        Vec3f newUp      = Normalize(Cross(newSide, newForward));
        m_qOrientation   = AngleAxis(newForward, 0.0f);
    }

    void RotateAroundUp(const float& angle)
    {
        Quat qRot      = AngleAxis(m_Up, angle);
        m_qOrientation = qRot * m_qOrientation;
        Mat4f rotMat   = AsMat4f(m_qOrientation);
        m_Forward      = Vec3f{ rotMat * Vec4f{ m_Forward } }; // TODO: Create 3x3 rotation matrix to avoid conversions.
        m_Up           = rotMat * m_Up;
    }

  private:
    Vec3f m_Position;
    Vec3f m_Forward;
    Vec3f m_Up;
    Quat  m_qOrientation;
};

Mat4f LookAt(const Vec3f& position, const Vec3f& target, const Vec3f& up)
{
    Vec3f t       = -position;
    Vec3f forward = Normalize(target - position);
    Vec3f right   = Cross(forward, Normalize(up));
    Vec3f newUp   = Cross(right, forward);
    Mat4f result  = Mat4f::Identity();
    result[ 0 ]   = Vec4f{ right, 0.0f };
    result[ 1 ]   = Vec4f{ newUp, 0.0f };
    result[ 2 ]   = Vec4f{ -forward, 0.0f };
    result.Transpose();
    result[ 3 ] = Vec4f{ t, 1.0f };
    return result;
}

/* Creates a right-handed, y-up, perspective projection matrix. */
Mat4f PerspectiveProjection(const float& fovy, const float& aspect, const float& near, const float& far)
{
    float d = 1.0f / tanf(fovy * 0.5f);
    float s = far - near;
    return Mat4f(d / aspect,
                 0.0f,
                 0.0f,
                 0.0f,
                 0.0f,
                 d,
                 0.0f,
                 0.0f,
                 0.0f,
                 0.0f,
                 -(near + far) / s,
                 -1.0f,
                 0.0f,
                 0.0f,
                 -(2.0f * near * far) / s,
                 0.0f);
}

Mat4f Translate(const Vec3f& v)
{
    Mat4f result = Mat4f::Identity();
    result[ 3 ]  = Vec4f{ v, 1.0f };

    return result;
}

Mat4f Scale(const Vec3f& v)
{
    Mat4f result = Mat4f::Identity();
    result[ 0 ][ 0 ] *= v.x;
    result[ 1 ][ 1 ] *= v.y;
    result[ 2 ][ 2 ] *= v.z;

    return result;
}

void Translate(Mat4f& m, const Vec3f& v)
{
    m[ 3 ][ 0 ] += v.x;
    m[ 3 ][ 1 ] += v.y;
    m[ 3 ][ 2 ] += v.z;
}

void Scale(Mat4f& m, const Vec3f& v)
{
    m[ 0 ][ 0 ] *= v.x;
    m[ 1 ][ 1 ] *= v.y;
    m[ 2 ][ 2 ] *= v.z;
}

static std::vector<Vertex> g_Cone;
static std::vector<Vertex> g_Cap;
static GLuint              g_ConeVBO;
static GLuint              g_CapVBO;

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
        float x = 50.0f * sinf(angle);
        float y = 50.0f * cosf(angle);

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
    Mat4f M1    = Mat4f::Identity();
    Mat4f M2    = Mat4f::Identity();
    Mat4f M1xM2 = M1 * M2;
    printf("M1xM2:\n%s\n", M1xM2.ToString());

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

    /* Init Dear ImGUI */
    /* ImGUI context */
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
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
    ImGui_ImplSDL3_InitForOpenGL(g_pWindow, g_glContext);
    ImGui_ImplOpenGL3_Init("#version 460");

    /* Load shaders. */
    Shader shader{};
    if ( !shader.Load("./shaders/task01.vert", "./shaders/task01.frag") )
    {
        fprintf(stderr, "Could not load shader.\n");
    }

    /* Create a model on GPU */
    Model model{};
    if ( !model.Load("./models/stormtrooper.obj") )
    {
        fprintf(stderr, "Could not load model file.\n");
    }

    /* Create Cylinder and upload to GPU */
    CreateGeometry();

    /* Create camera */
    Camera camera(Vec3f{ 0.0f, 20.0f, 10.0f }, Vec3f{ 0.0f, 0.0f, 0.0f });

    /* Model mat*/
    Mat4f modelMat = Scale(Vec3f{ 3.0f, 3.0f, 3.0f });
    Translate(modelMat, Vec3f{ 0.0f, 0.0f, -20.0f }); //Mat4f::Identity();

    /* View mat */
    Mat4f viewMat = LookAt(
        camera.GetPosition(), camera.GetPosition() + camera.GetForward(), camera.GetUp()); // Mat4f::Identity();
#if 1
    printf("viewMat: \n%s\n", viewMat.ToString());
    const float* viewMatData = viewMat.Data();
    printf("viewMatData: %f\n", viewMatData[ 0 ]);
    printf("viewMatData: %f\n", viewMatData[ 1 ]);
    printf("viewMatData: %f\n", viewMatData[ 2 ]);
    printf("viewMatData: %f\n", viewMatData[ 3 ]);
    printf("viewMatData: %f\n", viewMatData[ 4 ]);
    printf("viewMatData: %f\n", viewMatData[ 5 ]);
    printf("viewMatData: %f\n", viewMatData[ 6 ]);
    printf("viewMatData: %f\n", viewMatData[ 7 ]);
    printf("viewMatData: %f\n", viewMatData[ 8 ]);
    printf("viewMatData: %f\n", viewMatData[ 9 ]);
    printf("viewMatData: %f\n", viewMatData[ 10 ]);
    printf("viewMatData: %f\n", viewMatData[ 11 ]);
    printf("viewMatData: %f\n", viewMatData[ 12 ]);
    printf("viewMatData: %f\n", viewMatData[ 13 ]);
    printf("viewMatData: %f\n", viewMatData[ 14 ]);
    printf("viewMatData: %f\n", viewMatData[ 15 ]);
#endif

    /* VAO. */
    GLuint VAO;
    glCreateVertexArrays(1, &VAO);
    glVertexArrayVertexBuffer(VAO, 0, model.GetBuffer(), 0, sizeof(Vertex));
    glVertexArrayAttribBinding(VAO, 0, 0);
    glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glEnableVertexArrayAttrib(VAO, 0);

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
    glPointSize(20.0f);        // TODO: Delete later.
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
        SDL_GetWindowSize(g_pWindow, &windowWidth, &windowHeight);

        /* Adjust viewport and perspective projection accordingly. */
        glViewport(0, 0, windowWidth, windowHeight);
        /* Projection mat */
        float aspect  = (float)windowWidth / (float)windowHeight;
        Mat4f projMat = PerspectiveProjection(TO_RAD(70.0f), aspect, 0.01f, 500.0f);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();

        /* ImGUI Rendering */
        ImGui::Render();

        /* Rendering */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(1.0f, 0.95f, 0.0f, 1.0f);

        shader.Use();
        glBindVertexArray(VAO);
        glUniformMatrix4fv(0, 1, GL_FALSE, modelMat.Data());
        glUniformMatrix4fv(1, 1, GL_FALSE, viewMat.Data());
        glUniformMatrix4fv(2, 1, GL_FALSE, projMat.Data());
        glDrawArrays(GL_TRIANGLES, 0, model.NumVertices());

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(g_pWindow);
    }

    /* GL Resources shutdown. */
    shader.Delete();
    glDeleteVertexArrays(1, &VAO);

    /* ImGUI Shutdown */
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    /* SDL Shutdown */
    SDL_GL_DestroyContext(g_glContext);
    SDL_DestroyWindow(g_pWindow);
    SDL_Quit();

    return 0;
}
