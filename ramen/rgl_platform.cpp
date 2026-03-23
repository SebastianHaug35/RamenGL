#include "rgl_platform.h"

#include <stdlib.h>

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
