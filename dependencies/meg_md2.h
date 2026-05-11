#ifndef MEG_MD2_H
#define MEG_MD2_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define MEG_MD2_MAGIC_NUM (('I') + ('D' << 8) + ('P' << 16) + ('2' << 24))

    typedef struct meg_normal
    {
        float x, y, z;
    } meg_normal;

    typedef struct meg_md2_header
    {
        int identifier;  // identifies the file type, should be "IDP2"
        int version;     // version number, should be 8
        int skin_width;  // width of texture map used for skinning
        int skin_height; // height of texture map using for skinning
        int framesize;   // number of bytes in a single frame of animation
        int num_skins;   // total number of skins
        // listed by ASCII filename and are available
        // for loading if files are found in full path
        int num_verts; // number of vertices in each model frame, the
        // number of vertices in each frame is always the
        // same
        int num_textcoords; // total number of texture coordinates in entire file
        // may be larger than the number of vertices
        int num_polys; // number of polygons per model, or per frame of
        // animation if you will
        int num_openGLcmds; // number of openGL commands which can help with
        // rendering optimization
        // however, we won't be using them

        int num_frames; // total number of animation frames

        // memory byte offsets to actual data for each item

        int offset_skins; // offset in bytes from beginning of file to the
        // skin array that holds the file name for each skin,
        // each file name record is 64 bytes
        int offset_textcoords; // offset in bytes from the beginning of file to the
        // texture coordinate array

        int offset_polys; // offset in bytes from beginning of file to the
        // polygon mesh

        int offset_frames; // offset in bytes from beginning of file to the
        // vertex data for each frame

        int offset_openGLcmds; // offset in bytes from beginning of file to the
        // openGL commands

        int offset_end; // offset in bytes from beginning of file to end of file
    } meg_md2_header;

    typedef struct meg_vertex
    {
        float x, y, z;
        float nx, ny, nz;
    } meg_vertex;

    typedef struct meg_textcoord
    {
        float s, t;
    } meg_textcoord;

    typedef struct meg_md2_vertex
    {
        unsigned char x, y, z;
        unsigned char normal_index;
    } meg_md2_vertex;

    typedef struct meg_md2_frame
    {

        float scale[ 3 ];     // x,y,z scaling factors for the frame vertices
        float translate[ 3 ]; // x,y,z translation factors for the frame vertices
        char  name[ 16 ];     // ASCII name of the model, "evil death lord" etc. :)

        meg_md2_vertex vlist[ 1 ]; // beginning of vertex storage array

    } meg_md2_frame;

    typedef struct meg_md2_poly
    {
        uint16_t vindex[ 3 ]; // vertex indices
        uint16_t tindex[ 3 ]; // texture indices
    } meg_md2_poly;

    typedef struct meg_poly
    {
        uint16_t vindex[ 3 ];
        uint16_t tindex[ 3 ];
    } meg_poly;

    typedef struct meg_md2
    {
        meg_md2_header header;
        meg_vertex*    verts_array; /* Contains vertices of all frames, frame after frame. */
        uint16_t*      index_array;
        meg_textcoord* textcoords_array;
        meg_poly*      polys;
    } meg_md2;

    meg_md2* meg_md2_load_from_data(unsigned char* data);
    meg_md2* meg_md2_load_from_file(const char* file);
    void     meg_md2_free(meg_md2* p_md2);

    int meg_md2_framecount(meg_md2* md2);
    int meg_md2_skincount(meg_md2* md2);
    int meg_md2_vertexcount(meg_md2* md2);

#ifdef __cplusplus
}
#endif

///////////////////
// IMPLEMENTATION
///////////////////

#if defined(MEG_MD2_IMPLEMENTATION)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

meg_normal MEG_MD2_NORMALS[]
    = { { -0.525731, 0.000000, 0.850651 },   { -0.442863, 0.238856, 0.864188 },   { -0.295242, 0.000000, 0.955423 },
        { -0.309017, 0.500000, 0.809017 },   { -0.162460, 0.262866, 0.951056 },   { 0.000000, 0.000000, 1.000000 },
        { 0.000000, 0.850651, 0.525731 },    { -0.147621, 0.716567, 0.681718 },   { 0.147621, 0.716567, 0.681718 },
        { 0.000000, 0.525731, 0.850651 },    { 0.309017, 0.500000, 0.809017 },    { 0.525731, 0.000000, 0.850651 },
        { 0.295242, 0.000000, 0.955423 },    { 0.442863, 0.238856, 0.864188 },    { 0.162460, 0.262866, 0.951056 },
        { -0.681718, 0.147621, 0.716567 },   { -0.809017, 0.309017, 0.500000 },   { -0.587785, 0.425325, 0.688191 },
        { -0.850651, 0.525731, 0.000000 },   { -0.864188, 0.442863, 0.238856 },   { -0.716567, 0.681718, 0.147621 },
        { -0.688191, 0.587785, 0.425325 },   { -0.500000, 0.809017, 0.309017 },   { -0.238856, 0.864188, 0.442863 },
        { -0.425325, 0.688191, 0.587785 },   { -0.716567, 0.681718, -0.147621 },  { -0.500000, 0.809017, -0.309017 },
        { -0.525731, 0.850651, 0.000000 },   { 0.000000, 0.850651, -0.525731 },   { -0.238856, 0.864188, -0.442863 },
        { 0.000000, 0.955423, -0.295242 },   { -0.262866, 0.951056, -0.162460 },  { 0.000000, 1.000000, 0.000000 },
        { 0.000000, 0.955423, 0.295242 },    { -0.262866, 0.951056, 0.162460 },   { 0.238856, 0.864188, 0.442863 },
        { 0.262866, 0.951056, 0.162460 },    { 0.500000, 0.809017, 0.309017 },    { 0.238856, 0.864188, -0.442863 },
        { 0.262866, 0.951056, -0.162460 },   { 0.500000, 0.809017, -0.309017 },   { 0.850651, 0.525731, 0.000000 },
        { 0.716567, 0.681718, 0.147621 },    { 0.716567, 0.681718, -0.147621 },   { 0.525731, 0.850651, 0.000000 },
        { 0.425325, 0.688191, 0.587785 },    { 0.864188, 0.442863, 0.238856 },    { 0.688191, 0.587785, 0.425325 },
        { 0.809017, 0.309017, 0.500000 },    { 0.681718, 0.147621, 0.716567 },    { 0.587785, 0.425325, 0.688191 },
        { 0.955423, 0.295242, 0.000000 },    { 1.000000, 0.000000, 0.000000 },    { 0.951056, 0.162460, 0.262866 },
        { 0.850651, -0.525731, 0.000000 },   { 0.955423, -0.295242, 0.000000 },   { 0.864188, -0.442863, 0.238856 },
        { 0.951056, -0.162460, 0.262866 },   { 0.809017, -0.309017, 0.500000 },   { 0.681718, -0.147621, 0.716567 },
        { 0.850651, 0.000000, 0.525731 },    { 0.864188, 0.442863, -0.238856 },   { 0.809017, 0.309017, -0.500000 },
        { 0.951056, 0.162460, -0.262866 },   { 0.525731, 0.000000, -0.850651 },   { 0.681718, 0.147621, -0.716567 },
        { 0.681718, -0.147621, -0.716567 },  { 0.850651, 0.000000, -0.525731 },   { 0.809017, -0.309017, -0.500000 },
        { 0.864188, -0.442863, -0.238856 },  { 0.951056, -0.162460, -0.262866 },  { 0.147621, 0.716567, -0.681718 },
        { 0.309017, 0.500000, -0.809017 },   { 0.425325, 0.688191, -0.587785 },   { 0.442863, 0.238856, -0.864188 },
        { 0.587785, 0.425325, -0.688191 },   { 0.688191, 0.587785, -0.425325 },   { -0.147621, 0.716567, -0.681718 },
        { -0.309017, 0.500000, -0.809017 },  { 0.000000, 0.525731, -0.850651 },   { -0.525731, 0.000000, -0.850651 },
        { -0.442863, 0.238856, -0.864188 },  { -0.295242, 0.000000, -0.955423 },  { -0.162460, 0.262866, -0.951056 },
        { 0.000000, 0.000000, -1.000000 },   { 0.295242, 0.000000, -0.955423 },   { 0.162460, 0.262866, -0.951056 },
        { -0.442863, -0.238856, -0.864188 }, { -0.309017, -0.500000, -0.809017 }, { -0.162460, -0.262866, -0.951056 },
        { 0.000000, -0.850651, -0.525731 },  { -0.147621, -0.716567, -0.681718 }, { 0.147621, -0.716567, -0.681718 },
        { 0.000000, -0.525731, -0.850651 },  { 0.309017, -0.500000, -0.809017 },  { 0.442863, -0.238856, -0.864188 },
        { 0.162460, -0.262866, -0.951056 },  { 0.238856, -0.864188, -0.442863 },  { 0.500000, -0.809017, -0.309017 },
        { 0.425325, -0.688191, -0.587785 },  { 0.716567, -0.681718, -0.147621 },  { 0.688191, -0.587785, -0.425325 },
        { 0.587785, -0.425325, -0.688191 },  { 0.000000, -0.955423, -0.295242 },  { 0.000000, -1.000000, 0.000000 },
        { 0.262866, -0.951056, -0.162460 },  { 0.000000, -0.850651, 0.525731 },   { 0.000000, -0.955423, 0.295242 },
        { 0.238856, -0.864188, 0.442863 },   { 0.262866, -0.951056, 0.162460 },   { 0.500000, -0.809017, 0.309017 },
        { 0.716567, -0.681718, 0.147621 },   { 0.525731, -0.850651, 0.000000 },   { -0.238856, -0.864188, -0.442863 },
        { -0.500000, -0.809017, -0.309017 }, { -0.262866, -0.951056, -0.162460 }, { -0.850651, -0.525731, 0.000000 },
        { -0.716567, -0.681718, -0.147621 }, { -0.716567, -0.681718, 0.147621 },  { -0.525731, -0.850651, 0.000000 },
        { -0.500000, -0.809017, 0.309017 },  { -0.238856, -0.864188, 0.442863 },  { -0.262866, -0.951056, 0.162460 },
        { -0.864188, -0.442863, 0.238856 },  { -0.809017, -0.309017, 0.500000 },  { -0.688191, -0.587785, 0.425325 },
        { -0.681718, -0.147621, 0.716567 },  { -0.442863, -0.238856, 0.864188 },  { -0.587785, -0.425325, 0.688191 },
        { -0.309017, -0.500000, 0.809017 },  { -0.147621, -0.716567, 0.681718 },  { -0.425325, -0.688191, 0.587785 },
        { -0.162460, -0.262866, 0.951056 },  { 0.442863, -0.238856, 0.864188 },   { 0.162460, -0.262866, 0.951056 },
        { 0.309017, -0.500000, 0.809017 },   { 0.147621, -0.716567, 0.681718 },   { 0.000000, -0.525731, 0.850651 },
        { 0.425325, -0.688191, 0.587785 },   { 0.587785, -0.425325, 0.688191 },   { 0.688191, -0.587785, 0.425325 },
        { -0.955423, 0.295242, 0.000000 },   { -0.951056, 0.162460, 0.262866 },   { -1.000000, 0.000000, 0.000000 },
        { -0.850651, 0.000000, 0.525731 },   { -0.955423, -0.295242, 0.000000 },  { -0.951056, -0.162460, 0.262866 },
        { -0.864188, 0.442863, -0.238856 },  { -0.951056, 0.162460, -0.262866 },  { -0.809017, 0.309017, -0.500000 },
        { -0.864188, -0.442863, -0.238856 }, { -0.951056, -0.162460, -0.262866 }, { -0.809017, -0.309017, -0.500000 },
        { -0.681718, 0.147621, -0.716567 },  { -0.681718, -0.147621, -0.716567 }, { -0.850651, 0.000000, -0.525731 },
        { -0.688191, 0.587785, -0.425325 },  { -0.587785, 0.425325, -0.688191 },  { -0.425325, 0.688191, -0.587785 },
        { -0.425325, -0.688191, -0.587785 }, { -0.587785, -0.425325, -0.688191 }, { -0.688191, -0.587785, -0.425325 } };

meg_md2* meg_md2_load_from_data(unsigned char* data)
{
    /* Copy header data */
    meg_md2_header* header = (meg_md2_header*)data;
    meg_md2*        md2    = (meg_md2*)malloc(sizeof(meg_md2));
    memcpy(&(md2->header), header, sizeof(meg_md2_header));

    /* Store uncompressed vertices of all frames in a large array */
    md2->verts_array      = (meg_vertex*)malloc(header->num_verts * header->num_frames * sizeof(meg_vertex));
    meg_md2_frame* frames = (meg_md2_frame*)(data + header->offset_frames);
    for ( int i = 0; i < md2->header.num_frames; i++ )
    {
        meg_md2_frame*  pframe  = (meg_md2_frame*)((unsigned char*)frames + (i * header->framesize));
        meg_md2_vertex* vert    = pframe->vlist;
        meg_vertex*     megvert = md2->verts_array + (i * header->num_verts);
        for ( int v = 0; v < header->num_verts; v++ )
        {
            megvert[ v ].x             = pframe->scale[ 0 ] * (float)(vert->x) + pframe->translate[ 0 ];
            megvert[ v ].y             = pframe->scale[ 1 ] * (float)(vert->y) + pframe->translate[ 1 ];
            megvert[ v ].z             = pframe->scale[ 2 ] * (float)(vert->z) + pframe->translate[ 2 ];
            unsigned char normal_index = vert->normal_index;
            megvert[ v ].nx            = MEG_MD2_NORMALS[ normal_index ].x;
            megvert[ v ].ny            = MEG_MD2_NORMALS[ normal_index ].y;
            megvert[ v ].nz            = MEG_MD2_NORMALS[ normal_index ].z;
            vert++;
            // printf("%f, %f, %f\n", md2->verts_array[ v ].x, md2->verts_array[ v ].y, md2->verts_array[ v ].z); // DEBUG
        }
    }

    /* Iterate through polygons to build indexarray */
    meg_md2_poly* ppoly = (meg_md2_poly*)(data + header->offset_polys);
    md2->polys          = (meg_poly*)malloc(header->num_polys * sizeof(meg_poly));
    memcpy(md2->polys, ppoly, header->num_polys * sizeof(meg_poly));
    int num_total_indices = header->num_polys * 3;
    md2->index_array      = (uint16_t*)malloc(num_total_indices * sizeof(uint16_t));
    for ( int i = 0; i < header->num_polys; i++ )
    {
        meg_md2_poly poly      = ppoly[ i ];
        uint16_t     vertIdxA  = poly.vindex[ 0 ];
        uint16_t     vertIdxB  = poly.vindex[ 1 ];
        uint16_t     vertIdxC  = poly.vindex[ 2 ];
        uint16_t*    idx_array = md2->index_array + i * 3;
        idx_array[ 0 ]         = vertIdxA;
        idx_array[ 1 ]         = vertIdxB;
        idx_array[ 2 ]         = vertIdxC;
        printf("vindices: %d, %d, %d\n", idx_array[ 0 ], idx_array[ 1 ], idx_array[ 2 ]);
        printf("tindices: %d, %d, %d\n", poly.tindex[ 0 ], poly.tindex[ 1 ], poly.tindex[ 2 ]);
    }

    // TODO: Remove later
    /* Iterate through skin_names and print */
    printf("SKIN NAMES:\n");
    char* skins = (char*)(data + header->offset_skins);
    for ( int i = 0; i < header->num_skins; i++ )
    {
        printf("Skin %d: ", i);
        printf("%.*s\n", 64, skins);
        skins += 64;
    }

    /* Iterate through texture coordinates */
    uint16_t* texcoords      = (uint16_t*)(data + header->offset_textcoords);
    md2->textcoords_array    = (meg_textcoord*)malloc(header->num_textcoords * sizeof(meg_textcoord));
    meg_textcoord* textcoord = md2->textcoords_array;
    printf("TEXT COORDS:\n");
    for ( int i = 0; i < header->num_textcoords; i++ )
    {
        uint16_t s = texcoords[ 0 ];
        uint16_t t = texcoords[ 1 ];
        printf("%d, %d\n", s, t);
        float fS = (float)s / 255.9f;
        float fT = (float)t / 255.9f;
        printf("(%f, %f)\n", fS, fT);
        *textcoord = { fS, fT };
        texcoords += 2;
        textcoord++;
    }
    printf("TEXT COORDS ARRAY:\n");
    for ( int i = 0; i < header->num_textcoords; i++ )
    {
        printf("%f, %f\n", md2->textcoords_array[ i ].s, md2->textcoords_array[ i ].t);
    }

    /* Iterate through frames */
    printf("num_frames: %d\n", header->num_frames);
    for ( int i = 0; i < header->num_frames; i++ )
    {
        meg_md2_frame* frame = (meg_md2_frame*)(data + header->offset_frames + i * header->framesize);

        printf("---FRAME: %.*s---\n", 16, frame->name);
        float* scale     = frame->scale;
        float* translate = frame->translate;
        printf("scale: (%f, %f, %f), translate: (%f, %f, %f)\n",
               scale[ 0 ],
               scale[ 1 ],
               scale[ 2 ],
               translate[ 0 ],
               translate[ 1 ],
               translate[ 2 ]);
    }
    // exit(1);

    return md2;
}

meg_md2* meg_md2_load_from_file(const char* file)
{
    FILE* f = fopen(file, "rb");
    if ( !f )
    {
        return NULL;
    }

    fseek(f, 0L, SEEK_END);
    int len = ftell(f);
    rewind(f);
    unsigned char* data = (unsigned char*)malloc(len);
    int            ret  = fread(data, sizeof(*data), len, f);
    fclose(f);
    printf("file is %d bytes in size.\n", len); // DEBUG.

    return meg_md2_load_from_data(data);
}

void meg_md2_free(meg_md2* p_md2)
{
    free(p_md2);
}

int meg_md2_framecount(meg_md2* md2)
{
    return 0;
}
int meg_md2_skincount(meg_md2* md2)
{
    return 0;
}
int meg_md2_vertexcount(meg_md2* md2)
{
    return 0;
}

#endif

#endif // MEG_MD2_H
