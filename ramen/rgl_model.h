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
                for ( int k = 0; k < 3; k++ )
                {
                    v.position[ k ] = attrib.vertices[ 3 * idx.vertex_index + k ];
                    v.normal[ k ]   = attrib.normals[ 3 * idx.normal_index + k ];
                }
                m_Vertices.push_back(v);
            }
        }
        printf("m_Vertices.size(): %lu\n", m_Vertices.size());

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
