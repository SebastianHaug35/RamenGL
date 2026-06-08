#ifndef RGL_FILESYSTEM_H
#define RGL_FILESYSTEM_H

#include <physfs.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vector>

#include "rgl_platform.h"

class Filesystem
{
  public:
    static Filesystem* Init(int argc, char** argv, const char* baseDir = nullptr);
    static Filesystem* Instance();

    File Read(const char* filename)
    {
        File         result{};
        PHYSFS_File* hFile = PHYSFS_openRead(filename);

        if ( !hFile ) /* FAILURE */
        {
            fprintf(stderr, "Failed to read file: '%s'\n", filename);
            fprintf(stderr, "PHYSFS: %s\n", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
            return result;
        }

        int64_t len       = PHYSFS_fileLength(hFile);
        result            = CreateFile(len);
        int64_t bytesRead = PHYSFS_readBytes(hFile, result.data, len);

        if ( bytesRead < len )
        {
            fprintf(stderr, "bytes read: %ld, file length: %ld\n", bytesRead, len);
            if ( bytesRead == -1 ) /* FAILURE! */
            {
                fprintf(stderr, "PHYSFS: %s\n", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
                return result;
            }
        }

        PHYSFS_close(hFile);

        return result;
    }

    char** GetFiles(const char* dir, size_t* numFolders) const
    {
        char** result = (char**)malloc(sizeof(char*));
        char** files  = PHYSFS_enumerateFiles(dir);
        char** f      = files;
        size_t i      = 0;
        while ( *f )
        {
            const char* file     = *f;
            size_t      len      = strlen(file);
            char*       filename = (char*)malloc(len * sizeof(char));
            snprintf(filename, len + 1, "%s", file);
            result      = (char**)realloc(result, sizeof(char*) * (i + 1));
            result[ i ] = filename;
            f++;
            i++;
        }
        result      = (char**)realloc(result, sizeof(char*) * (i + 1));
        result[ i ] = nullptr;
        *numFolders = i;

        PHYSFS_freeList(files);

        return result;
    }

  private:
    Filesystem() = default;
};

#endif
