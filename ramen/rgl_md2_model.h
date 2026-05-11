#ifndef RGL_MD2_MODEL_H
#define RGL_MD2_MODEL_H

#include <vector>

#include <stdint.h>

#include <meg_md2.h>

#include "rgl_filesystem.h"
#include "rgl_model.h"

class Md2Model
{
  public:
    bool Load(const char* file)
    {
        File modelFile = Filesystem::Instance()->Read(file);
        if ( !modelFile.data )
        {
            return false;
        }

        meg_md2* pMd2 = meg_md2_load_from_data((unsigned char*)modelFile.data);

        PrintHeader(&pMd2->header);
        // for ( int i = 0; i < pMd2->header.num_frames * pMd2->header.num_verts; i++ )
        // {
        //     printf("%f, %f, %f\n", pMd2->verts_array[ i ].x, pMd2->verts_array[ i ].y, pMd2->verts_array[ i ].z);
        // }

        meg_vertex* pV = pMd2->verts_array;
        for ( int i = 0; i < pMd2->header.num_verts * pMd2->header.num_frames; i++ )
        {
            // uint16_t uv_index = pV[ i ].uv_index;
            // meg_textcoord textcoord = pMd2->textcoords_array[ uv_index ];
            // printf("index: %d, s: %f, t: %f\n", uv_index, textcoord.s, textcoord.t);
            m_Vertices.push_back({ .position = Vec3f{ pV[ i ].x, pV[ i ].y, pV[ i ].z },
                                   .normal   = { pV[ i ].nx, pV[ i ].ny, pV[ i ].nz },
                                   .color    = Vec3f{ 0.0f } });
            // printf("%s\n", m_Vertices[ i ].position.ToString());
            // printf("%s\n", m_Vertices[ i ].uv.ToString());
        }
        printf("Getting verts. Done.\n");

        // printf("loading indices:\n");
        // printf("num polys: %d\n", pMd2->header.num_polys);

        for ( int i = 0; i < pMd2->header.num_frames; i++ )
        {
            size_t offset_into_vertsarray = size_t(i * pMd2->header.num_verts);
            for ( int j = 0; j < pMd2->header.num_polys; j++ )
            {
                const meg_poly&      poly    = pMd2->polys[ j ];
                const size_t         v0      = poly.vindex[ 0 ] + offset_into_vertsarray;
                const size_t         v1      = poly.vindex[ 1 ] + offset_into_vertsarray;
                const size_t         v2      = poly.vindex[ 2 ] + offset_into_vertsarray;
                const uint16_t&      uv_idx0 = poly.tindex[ 0 ];
                const uint16_t&      uv_idx1 = poly.tindex[ 1 ];
                const uint16_t&      uv_idx2 = poly.tindex[ 2 ];
                const meg_textcoord& t0      = pMd2->textcoords_array[ uv_idx0 ];
                const meg_textcoord& t1      = pMd2->textcoords_array[ uv_idx1 ];
                const meg_textcoord& t2      = pMd2->textcoords_array[ uv_idx2 ];
                m_Vertices[ v0 ].uv          = Vec3f{ t0.s, t0.t, 0.0f };
                m_Vertices[ v1 ].uv          = Vec3f{ t1.s, t1.t, 0.0f };
                m_Vertices[ v2 ].uv          = Vec3f{ t2.s, t2.t, 0.0f };
            }
        }

        uint16_t* pI = pMd2->index_array;
        for ( int i = 0; i < pMd2->header.num_polys * 3; i += 3 )
        {
            const uint16_t& v0 = pI[ i + 0 ];
            const uint16_t& v1 = pI[ i + 1 ];
            const uint16_t& v2 = pI[ i + 2 ];
            m_Indices.push_back(v0);
            m_Indices.push_back(v1);
            m_Indices.push_back(v2);
            // printf("%d, ", pI[ i + 0 ]);
            // printf("%d, ", pI[ i + 1 ]);
            // printf("%d\n", pI[ i + 2 ]);
        }

        m_NumFrames     = pMd2->header.num_frames;
        m_NumVerts      = pMd2->header.num_verts;
        m_NumTextCoords = pMd2->header.num_textcoords;
        m_NumSkins      = pMd2->header.num_skins;
        m_NumPolys      = pMd2->header.num_polys;
        m_FrameSize     = m_NumVerts * sizeof(Vertex);

        modelFile.Destroy();

        return true;
    }

    void PrintHeader(const meg_md2_header* pHeader)
    {
        printf("identifier: %d\n", pHeader->identifier);
        printf("version: %d\n", pHeader->version);
        printf("skin_width: %d\n", pHeader->skin_width);
        printf("skin_height: %d\n", pHeader->skin_height);
        printf("framesize: %d\n", pHeader->framesize);
        printf("num_skins: %d\n", pHeader->num_skins);
        printf("num_verts: %d\n", pHeader->num_verts);
        printf("num_textcoords: %d\n", pHeader->num_textcoords);
        printf("num_polys: %d\n", pHeader->num_polys);
        printf("num_openGLcmds: %d\n", pHeader->num_openGLcmds);
        printf("num_frames: %d\n", pHeader->num_frames);
        printf("offset_skins: %d\n", pHeader->offset_skins);
        printf("offset_textcoords: %d\n", pHeader->offset_textcoords);
        printf("offset_polys: %d\n", pHeader->offset_polys);
        printf("offset_frames: %d\n", pHeader->offset_frames);
        printf("offset_openGLcmds: %d\n", pHeader->offset_openGLcmds);
        printf("offset_end: %d\n", pHeader->offset_end);
    }

    const std::vector<Vertex>& GetVertices() const
    {
        return m_Vertices;
    }

    const std::vector<uint16_t>& GetIndices() const
    {
        return m_Indices;
    }

    const size_t NumVertices() const
    {
        return m_Vertices.size();
    }

    const size_t NumTextCoords() const
    {
        return m_NumTextCoords;
    }

    const size_t NumVerticesPerFrame() const
    {
        return m_NumVerts;
    }

    const size_t NumIndices() const
    {
        return m_Indices.size();
    }

    const size_t NumFrames() const
    {
        return m_NumFrames;
    }

  private:
  private:
    std::vector<Vertex>   m_Vertices;
    std::vector<uint16_t> m_Indices;

    size_t m_NumFrames;
    size_t m_NumVerts;
    size_t m_NumTextCoords;
    size_t m_NumSkins;
    size_t m_NumPolys;

    size_t m_FrameSize; /* NOTE: size in terms of struct Vertex and not what was in md2-file! */
};

#endif
