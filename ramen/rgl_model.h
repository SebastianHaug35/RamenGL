#ifndef RGL_MODEL_H
#define RGL_MODEL_H

#include <string>

#include <glad/glad.h>

#include <tiny_obj_loader.h>

#include "rgl_filesystem.h"
#include "rgl_math.h"
#include "rgl_platform.h"

struct Vertex
{
    Vec3f position;
    Vec3f normal;
    Vec3f color;
    Vec3f uv;
    Vec3f tangent;
    Vec3f bitangent;
};

class Model
{
  public:
    bool Load(const char* file)
    {
        File modelFile = Filesystem::Instance()->Read(file);
        if ( !modelFile.data )
        {
            return false;
        }

        tinyobj::ObjReader       reader;
        tinyobj::ObjReaderConfig reader_config;
        reader_config.mtl_search_path = ""; // not needed if no MTL
        if ( !reader.ParseFromString(std::string(modelFile.data), std::string("")) )
        {
            if ( !reader.Error().empty() )
            {
                fprintf(stderr, "tinyobjloader: %s\n", reader.Error().c_str());
            }
            return false;
            modelFile.Destroy();
        }

        const tinyobj::attrib_t& attrib = reader.GetAttrib();
        m_Shapes                        = reader.GetShapes();

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
                /* UV */
                if ( idx.texcoord_index >= 0 )
                {
                    v.uv.x = attrib.texcoords[ 2 * idx.texcoord_index ];
                    v.uv.y = attrib.texcoords[ 2 * idx.texcoord_index + 1 ];
                }
                else
                    v.uv = Vec3f{ 0.0f };
                for ( int k = 0; k < 3; k++ )
                {
                    /* Position */
                    v.position[ k ] = attrib.vertices[ 3 * idx.vertex_index + k ];

                    /* Normal */
                    if ( idx.normal_index >= 0 )
                        v.normal[ k ] = attrib.normals[ 3 * idx.normal_index + k ];
                    else
                        v.normal[ k ] = 0.0f;
                }
                m_Vertices.push_back(v);
            }
        }

        if ( attrib.normals.empty() )
        {
            for ( size_t i = 0; i + 2 < m_Vertices.size(); i += 3 )
            {
                const Vec3f edgeAB = m_Vertices[ i + 1 ].position - m_Vertices[ i ].position;
                const Vec3f edgeAC = m_Vertices[ i + 2 ].position - m_Vertices[ i ].position;
                const Vec3f faceNormal = Normalize(Cross(edgeAB, edgeAC));

                m_Vertices[ i ].normal = faceNormal;
                m_Vertices[ i + 1 ].normal = faceNormal;
                m_Vertices[ i + 2 ].normal = faceNormal;
            }
        }

        printf("m_Vertices.size(): %lu\n", m_Vertices.size());

        /* Compute Tangent / Bitangent */
#if 1
        for ( int i = 0; i < m_Vertices.size(); i += 3 )
        {
            Vertex&      v0 = m_Vertices[ i + 0 ];
            Vertex&      v1 = m_Vertices[ i + 1 ];
            Vertex&      v2 = m_Vertices[ i + 2 ];
            const Vec3f& p0 = v0.position;
            const Vec3f& p1 = v1.position;
            const Vec3f& p2 = v2.position;
            const Vec3f  e1 = p1 - p0;
            const Vec3f  e2 = p2 - p0;
            const Vec3f& t0 = v0.uv;
            const Vec3f& t1 = v1.uv;
            const Vec3f& t2 = v2.uv;

            const float x1 = t1.x - t0.x;
            const float y1 = t1.y - t0.y;
            const float x2 = t2.x - t0.x;
            const float y2 = t2.y - t0.y;

            const Mat2f uvFrame    = Mat2f{ x1, y1, x2, y2 };
            const float uvFrameDet = uvFrame.Det();

            Vec3f tangent   = Vec3f{ e1.x * y2 + e2.x * -y1, e1.y * y2 + e2.y * -y1, e1.z * y2 + e2.z * -y1 };
            Vec3f bitangent = Vec3f{ e1.x * -x2 + e2.x * x1, e1.y * -x2 + e2.y * x1, e1.z * -x2 + e2.z * x1 };
            tangent *= uvFrameDet;
            bitangent *= uvFrameDet;

            v0.tangent   = tangent;
            v1.tangent   = tangent;
            v2.tangent   = tangent;
            v0.bitangent = bitangent;
            v1.bitangent = bitangent;
            v2.bitangent = bitangent;
        }

        /* Normalize tangents / bitangents */
        //  TODO: Orthonormalize.
        for ( int i = 0; i < m_Vertices.size(); i++ )
        {
            Vertex& v = m_Vertices[ i ];
            v.tangent.Normalize();
            v.bitangent.Normalize();
        }
#endif

        modelFile.Destroy();

        return true;
    }

    const std::vector<Vertex>& GetVertices() const
    {
        return m_Vertices;
    }

    const size_t NumVertices()
    {
        return m_Vertices.size();
    }

  private:
  private:
    std::vector<tinyobj::shape_t> m_Shapes;
    std::vector<Vertex>           m_Vertices;
};

#endif
