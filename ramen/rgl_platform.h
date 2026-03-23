#ifndef RGL_PLATFORM_H
#define RGL_PLATFORM_H

#include <stdio.h>
#include <stdlib.h>

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

bool FileExists(const char* file);
File ReadFile(const char* file);

#endif
