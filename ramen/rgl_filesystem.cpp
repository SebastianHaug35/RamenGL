#include "rgl_filesystem.h"

#include <assert.h>
#include <string.h>

static Filesystem* pTheOneAndOnly;

Filesystem* Filesystem::Init(int argc, char** argv, const char* baseDir)
{
    if ( !pTheOneAndOnly )
    {
        printf("Init filesystem...\n");

        pTheOneAndOnly = new Filesystem();
        PHYSFS_init(argv[ 0 ]);
        char sysBaseDir[ 512 ];
        memset(sysBaseDir, 0, 512);
        bool pathSet = false;

        if ( argc > 1 )
        {
            strcpy(sysBaseDir, argv[ 1 ]);
            pathSet = true;
        }
        if ( !pathSet && baseDir )
        {
            const char* appDir = PHYSFS_getBaseDir();
            strcpy(sysBaseDir, appDir);
            strcat(sysBaseDir, baseDir);
            pathSet = true;
        }
        if ( !pathSet )
        {
            const char* appDir = PHYSFS_getBaseDir();
            strcpy(sysBaseDir, appDir);
        }
        printf("Mounting system directory: '%s'\n", sysBaseDir);

        if ( !PHYSFS_mount(sysBaseDir, "", 1) )
        {
            printf("Failed to mount: %s\n", sysBaseDir);
        }
        else
        {
            printf("Mounted system directory: %s\n", sysBaseDir);
            printf("Files available:\n");
            char** files = PHYSFS_enumerateFiles("");
            for ( char** i = files; *i != NULL; i++ )
            {
                printf("  %s\n", *i);
            }
            PHYSFS_freeList(files);
        }
    }

    return pTheOneAndOnly;
}

Filesystem* Filesystem::Instance()
{
    assert(pTheOneAndOnly && "Filesystem has not been initialized! Did you forget to call Filesystem::Init()?");
    return pTheOneAndOnly;
}
